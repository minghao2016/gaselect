//
//  PLSEvaluator.cpp
//  GenAlgPLS
//
//  Created by David Kepplinger on 03.05.2013.
//
//

#include "config.h"

#include <algorithm>
#include "PLSEvaluator.h"

#ifdef ENABLE_DEBUG_VERBOSITY
#define IF_DEBUG(expr) if(this->verbosity >= DEBUG_VERBOSE) { expr; }
#define IF_FULLY_VERBOSE(expr) if(this->verbosity >= FULLY_VERBOSE) { expr; }
#else
#define IF_DEBUG(expr)
#define IF_FULLY_VERBOSE(expr)
#endif

PLSEvaluator::PLSEvaluator(PLS* pls, const uint16_t numReplications, const uint16_t numSegments, const std::vector<uint32_t> &seed, const VerbosityLevel verbosity) :
Evaluator(verbosity), numReplications(numReplications), numSegments(numSegments),
nrows(pls->getNumberOfObservations()), segmentLength(nrows / numSegments),
completeSegments(nrows % numSegments), seed(seed), pls(pls), cloned(false)
{
	if(pls->getNumberOfResponseVariables() > 1) {
		throw std::invalid_argument("PLS evaluator only available for models with 1 response variable");
	}

	this->initRowNumbers();
};

double PLSEvaluator::evaluate(arma::uvec &columnSubset) {
	uint16_t maxNComp = ((columnSubset.n_elem < (this->nrows - 2 * this->segmentLength - 2)) ? columnSubset.n_elem : this->nrows - 2 * this->segmentLength - 2);
	double sumSEP = 0;
	arma::uword rep = 0;
	
	this->pls->setSubmatrixViewColumns(columnSubset);
	for(rep = 0; rep < this->numReplications; ++rep) {
		sumSEP += this->estSEP(maxNComp, this->shuffledRowNumbers[rep]);
	}

	IF_DEBUG(Rcpp::Rcout << "EVALUATOR: Sum of SEP:" << std::endl << sumSEP << std::endl)
	return -sumSEP;
}

inline void PLSEvaluator::initRowNumbers() {
	RNG rng(this->seed);
	ShuffledSet rowNumbers(this->nrows);
	this->shuffledRowNumbers.reserve(this->numReplications);

	for(uint16_t i = 0; i < this->numReplications; ++i) {
		this->shuffledRowNumbers.push_back(rowNumbers.shuffleAll(rng));
	}
}

double PLSEvaluator::estSEP(uint16_t ncomp, std::vector<arma::uword> &rowNumbers) {
	// (online) Sum of squares of differences from the (current) mean (residMeans)
	// M_2,n = sum( (x_i - mean_n) ^ 2 )
	arma::vec RSS(ncomp);
	double RSSSDdelta = 0.0;
	arma::vec RSSSDm2n(ncomp);
	std::vector<double> RSSSDmean(ncomp, 0.0);
	double r2;

	uint16_t seg = 0, comp = 0;
	arma::uword n = 0, nSeg = 0;

	// If not all segments are the same length, segmentLength is the longer one
	// (i.e. the incomplete segments will be one element shorter)
	arma::uword segmentLength = this->segmentLength;
	arma::sword completeSegments = this->completeSegments;
	arma::uword lastSegmentLength = segmentLength; // if not all segments are the same length, the last segment is always one short

	arma::mat residuals;
	arma::mat leftOutX;
	arma::mat leftOutY;
	
	RSSSDm2n.zeros();
	RSS.zeros();

	if(this->completeSegments > 0) {
		++segmentLength;
	}
	
	/*
	 * the last segment is used as test set, and is excluded from all other calculations
	 */
	IF_FULLY_VERBOSE(arma::urowvec(rowNumbers).raw_print(Rcpp::Rcout, "EVALUATOR - Shuffled row numbers:");)

	for(; seg < this->numSegments - 1; ++seg) {
		arma::uvec segment(&rowNumbers[0], segmentLength, false);
		arma::uvec notSegment(&rowNumbers[segmentLength], this->nrows - segmentLength - lastSegmentLength, false);
		
		if(--completeSegments == 0) {
			--segmentLength;
		}
		
		IF_FULLY_VERBOSE(
			segment.t().raw_print(Rcpp::Rcout, "EVALUATOR - Segment");
			notSegment.t().raw_print(Rcpp::Rcout, "EVALUATOR - Remaining");
		)

		/*
		 * Segment the data and fit the PLS model with up to
		 * ncomp components
		 */
		
		leftOutX = this->pls->getXColumnView().rows(segment);
		leftOutY = this->pls->getY().rows(segment);
		this->pls->setSubmatrixViewRows(notSegment, true);

		this->pls->fit(ncomp);

		/*
		 * Calculate the standard error for observations not present in this segment
		 * and the RSS (in order to calculate the mean squared error)
		 */
		for(comp = 0; comp < ncomp; ++comp) {
			residuals = arma::square(leftOutY - this->pls->predict(leftOutX, comp + 1));
			
			/* Only consider multiple PLS (not multivariate), i.e. only use first response vector */
			for (nSeg = 0; nSeg < residuals.n_rows; ++nSeg) {
				r2 = residuals[nSeg] * residuals[nSeg];
				RSS[comp] += r2;
				
				RSSSDdelta = r2 - RSSSDmean[comp];
				RSSSDmean[comp] = RSSSDmean[comp] + RSSSDdelta / (n + 1 + nSeg);
				RSSSDm2n[comp] = RSSSDm2n[comp] + RSSSDdelta * (r2 - RSSSDmean[comp]);
			}
		}

		n += nSeg;
	}

	/*
	 * Find best number of components based on the RSS and one standard deviation
	 */
	double cutoff = RSS[0];
	arma::uword Aopt = 0;
	
	for(comp = 1; comp < ncomp; ++comp) {
		if(RSS[comp] < cutoff) {
			Aopt = comp;
			cutoff = RSS[comp];
		}
	}
	
	cutoff += sqrt(RSSSDm2n[Aopt] / (this->nrows - segmentLength));
	
	Aopt = 0;
	while(RSS[Aopt] > cutoff) {
		++Aopt;
		if(Aopt > ncomp) {
			Aopt = 0;
			break;
		}
	}
	++Aopt;
	
	/*
	 * The last segment is used as test set
	 */

	arma::uvec segment(&rowNumbers[this->nrows - lastSegmentLength], lastSegmentLength, false);
	arma::uvec notSegment(&rowNumbers[0], this->nrows - lastSegmentLength, false);
	
	IF_FULLY_VERBOSE(
		segment.t().raw_print(Rcpp::Rcout, "EVALUATOR - Validation Segment");
		notSegment.t().raw_print(Rcpp::Rcout, "EVALUATOR - Validation Remaining");
	)
	
	leftOutX = this->pls->getXColumnView().rows(segment);
	leftOutY = this->pls->getY().rows(segment);
	this->pls->setSubmatrixViewRows(notSegment, true);
	this->pls->fit(Aopt);
	residuals = leftOutY - this->pls->predict(leftOutX, Aopt);
	
	IF_FULLY_VERBOSE(Rcpp::Rcout << "EVALUATOR: Resulting SEP:" << std::endl << arma::stddev(residuals.col(0)) << std::endl)
	
	return arma::stddev(residuals.col(0));
}

Evaluator* PLSEvaluator::clone() const {
	PLSEvaluator* that = new PLSEvaluator(this->pls->clone(), this->numReplications, this->numSegments, this->seed, this->verbosity);
	that->cloned = true;
	return that;	
}



