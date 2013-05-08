//
//  Population.h
//  
//

#ifndef _Population_h
#define _Population_h

#include <iostream>
#include <vector>
#include <set>

#include "Chromosome.h"
#include "Evaluator.h"
#include "Control.h"

class Population {

public:
	Population(const Control &ctrl, const ::Evaluator &evaluator);
	~Population();

	void run();

	struct ChromosomeComparator {
		bool operator() (const Chromosome *lhs, const Chromosome *rhs) const {
			return rhs->isFitterThan(*lhs);
		}
	};
	
	/**
	 * Returns the last generation and the elite (if any)
	 * Invalid once the Population object is destroyed!
	 */
	std::multiset<Chromosome*, Population::ChromosomeComparator> getResult() const;
	
private:
	std::multiset<Chromosome*, Population::ChromosomeComparator> elite;
	std::vector<Chromosome*> currentGeneration;
	std::vector<double> fitnessMap;
	double minEliteFitness;
	
//	double evaluateFitness(Chromosome* ch);
	Chromosome* getChromosomeFromFitnessMap(double rand) const;
	void addChromosomeToElite(Chromosome* ch);
	
	std::ostream& printChromosomeFitness(std::ostream &os, Chromosome *ch);

	const Control ctrl;
	const Evaluator * const evaluator;
};

typedef std::multiset<Chromosome*, Population::ChromosomeComparator> SortedChromosomes;

#endif
