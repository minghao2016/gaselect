//
//  PLSEvaluator.h
//  GenAlgPLS
//
//  Created by David Kepplinger on 03.05.2013.
//
//

#ifndef GenAlgPLS_PLSEvaluator_h
#define GenAlgPLS_PLSEvaluator_h

#include "config.h"

#include <RcppArmadillo.h>
#include "UnifGenerator_0_1.h"

#include "Evaluator.h"
#include "Chromosome.h"
#include "PLS.h"

class PLSEvaluator : public Evaluator {
public:
	PLSEvaluator(PLS* pls, const uint16_t numReplications, const uint16_t numSegments, const VerbosityLevel verbosity, UnifGenerator_0_1* unifGen) :
	Evaluator(verbosity), numReplications(numReplications), numSegments(numSegments),
	nrows(pls->getNumberOfObservations()), segmentLength(nrows / numSegments),
	completeSegments(nrows % numSegments), pls(pls), unifGen(unifGen), cloned(false)
	{
		if(pls->getNumberOfResponseVariables() > 1) {
			throw Rcpp::exception("PLS evaluator only available for models with 1 response variable", __FILE__, __LINE__);
		}
	};

	~PLSEvaluator() {
		if(this->cloned == true) {
			delete this->pls;
		}
	}
	
	double evaluate(Chromosome &ch) const;
	double evaluate(arma::uvec &colSubset) const;

	void setUnifGenerator(UnifGenerator_0_1 *unifGen) {
		this->unifGen = unifGen;
	};
	
	Evaluator* clone() const;

private:
	const uint16_t numReplications;
	const uint16_t numSegments;
	const arma::uword nrows;
	const arma::uword segmentLength; // The length of the incomplete segments
	const uint16_t completeSegments; // The number of segments with `segmentLength` + 1 elements. If 0, all segments have `segmentLength` elements

	PLS *pls;
	UnifGenerator_0_1 *unifGen;

	bool cloned;

	/**
	 * Doesn't calculate the actual SEP but
	 * only square root of the sum of squared differences (SSD)
	 * SEP = SSD / sqrt(n - 1)
	 */
	arma::vec calcSSD(uint16_t ncomp, arma::uvec &rowNumbers) const;

	arma::uvec initRowNumbers() const;
};


#endif
