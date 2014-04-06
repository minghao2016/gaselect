//
//  PLS.h
//

#ifndef GenAlgPLS_PLS_h
#define GenAlgPLS_PLS_h

#include "config.h"

#include <RcppArmadillo.h>

enum PLSMethod {
	SIMPLS = 0
};

class PLS {
protected:
	enum ViewState {
		UNKNOWN = 0,
		COLUMNS,
		ROWS
	};

public:
	PLS(const arma::mat &X, const arma::mat &Y) : X(X), Y(Y), currentViewState(UNKNOWN) {};
	virtual ~PLS() {};

	static PLS* getInstance(PLSMethod method, const arma::mat &X, const arma::mat &Y);

	/**
	 * Reset the current view to be the original X and original Y matrix
	 * and then select the given columns from the X matrix
	 */
	virtual void viewSelectColumns(const arma::uvec &columns);

	/**
	 * Select the given rows from the current column-view for further
	 * processing
	 */
	virtual void viewSelectRows(const arma::uvec &rows);


	/**
	 * Fit a PLS model to the data with the previously set view
	 * with up to ncomp components
	 */
	virtual void fit(uint16_t ncomp = 0) =0;

	/**
	 * Get the coefficients of the last fit (i.e. coefficients
	 * that are obtained with ncomp specified in the last call
	 * to PLS::fit).
	 */
	virtual const arma::cube& getCoefficients() const = 0;

	/**
	 * Returns the intercept term for every number of components
	 * i.e. ncomp x nresp matrix
	 */
	virtual const arma::mat& getIntercepts() const = 0;

	/**
	 * Get dimensions of original matrices Y and X
	 */
	arma::uword getNumberOfPredictorVariables() const { return this->X.n_cols; }
	arma::uword getNumberOfResponseVariables() const { return this->Y.n_cols; }
	arma::uword getNumberOfObservations() const { return this->X.n_rows; }

	/**
	 * Returns the number components the last fit was performed with
	 */
	uint16_t getResultNComp() const { return this->resultNComp; }

	/**
	 * ncomp should be 0 based (as in the fit method)
	 */
	arma::mat predict(const arma::mat &newX, uint16_t ncomp) const;
	arma::cube predict(const arma::mat &newX) const;
	
	const arma::mat & getXColumnView() const { return this->viewXCol; }
	const arma::mat & getY() const { return this->Y; }
	
	virtual PLS* clone() const = 0;
	
protected:
	const arma::mat X;
	const arma::mat Y;

	uint16_t resultNComp;

	ViewState currentViewState;
	arma::mat viewXCol;
	arma::mat viewY;
	arma::mat viewX;
};

#include "PLSSimpls.h"

#endif
