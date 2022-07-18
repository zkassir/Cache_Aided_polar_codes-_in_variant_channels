#ifndef CONFLICTGRAPH_H
#define CONFLICTGRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <bits/stdc++.h>
#include "randomHandler.h"
#include "CheckFunction.h"
#include "DataDefinition.h"

/* Include GSL header files. */
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

namespace caching{

void _dealloc();

void computeNumberOfNodes();

void makeNodes();
void makeEdges();

int *colorUpdate();
int nCol();

void splitNode(int, int );
void MergeNode(int, int);

void makeEdges_ON_node_list();

cf_data conflictGraphGenerator(data_matrix data, vector<int>);
cf_data conflictGraphGeneratorUpdated(data_matrix data, vector<int> , vector<int>, int * );

}//end namespace caching

#endif // CONFLICTGRAPH_H
