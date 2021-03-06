/* Author: Roosevelt Young
* Institution: Washington State University
* */
//#include "stdafx.h"
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h> 
#include <string.h>

#pragma warning(disable:4996)

int *countEdge;
int *countWalks;
/*
* Used in qsort, sorts by the number of times a node has been walked to
*/
int comp(const void * elem1, const void * elem2)
{
	int f = *((int*)elem1);
	int s = *((int*)elem2);
	if (countWalks[f] < countWalks[s]) return  1;
	if (countWalks[f] > countWalks[s]) return -1;
	return 0;
}

/*
* Returns the number of nodes that exist 
*/
int getnNodes(char* argv[] )
{
	int nNodes = 0;
	FILE *graph = fopen(argv[3], "r");
	char line[100];
	int node = 0;
	int i = 0;
	int prevNode = 0;
	while (!feof(graph)) {
		if (fgets(line, sizeof(line), graph)) {
			while (line[i] != ' ')
			{
				i++;
			}
			line[i] = '\0';
		}
	}
	i = 0;
	fclose(graph);
	sscanf(line, "%d", &node);
	return node;
}
/*
*Returns the number of edges each node has
*/
int getmNodes(char* argv[], int nNodes)
{
	FILE *graph = fopen(argv[3], "r");
	char line[100];
	int node = 0;
	int i = 0;
	int prevNode = -1;

	countEdge = (int*)malloc(sizeof(int) * nNodes);
	while (!feof(graph)) {
		if (fgets(line, sizeof(line), graph)) {
			while (line[i] != ' ')
			{
				i++;
			}
			line[i] = '\0';
			sscanf(line, "%d", &node);
			if (node != prevNode)
			{
				countEdge[node] = 0;
			}
			countEdge[node]++;
			prevNode = node;
		}
	}
	i = 0;
	fclose(graph);
	return node;
}
/*
*Initializes the graph into memeory
*Uses getnNodes and getmNodes to create a 2d array of proper size and inserts data from the given file
*/
int** initGraph(char* argv[], int nNodes)
{
	FILE *graph = fopen(argv[3], "r");
	char line[100];
	char* tmp;
	int node = 0;
	int i = 0;
	int prevNode = -1;
	int edge = 0;
	int **graphnodes = (int**)malloc(sizeof(int*) * nNodes);

	while (!feof(graph)) {
		if (fgets(line, sizeof(line), graph)) {
			tmp = strtok(line, " ");
			sscanf(tmp, "%d", &node);
			tmp = strtok(NULL, " ");
			sscanf(tmp, "%d", &edge);
			if (node != prevNode)
			{
				graphnodes[node] = (int*)malloc(sizeof(int)*countEdge[node]);
				i = 0;
			}
			graphnodes[node][i] = edge;
			prevNode = node;
			i++;
		}
	}
	i = 0;
	fclose(graph);
	return graphnodes;
}
/*
* A parallel pagerank walker 
* High Level: The input is a direct graph G(V,E) with n verices and M edges, Vertices represent webpages.
* Starting from every node in the graph, a random walk is preformed starting from that point of length K,
* the amount of times each node is visited is kept track in countWalks[n] where n is the node, this gives an
* estimate on the amount of times a page is visited.
* 
* In order to do a random walk, we use D as a probability to see if we should walk to a edge or to 
* a completely random node in the fashion of:
*
* Toss a coin the probability D
* 
* If the toss = tail, then target = a node that is in the list of edges
* else target = a node that is a random node in the list of nodes 
*/
int walk(int **graph, int nNodes, int p, int D, int K)
{
	int currNode = 0;
	int k_Index = 0;
	int nextNode = 0;
	int g;
	int index;
	int indexGraph = 0;
	int node = 0;
	omp_set_num_threads(p);
	countWalks = (int*)malloc(sizeof(int)*nNodes);
	for (nextNode = 0; nextNode < nNodes; nextNode++)
	{
		countWalks[nextNode] = 0;
	}
	double timeCount = omp_get_wtime();

#pragma omp parallel private(currNode, k_Index, nextNode, g, index, indexGraph, node) shared(p)
	{
		index = omp_get_thread_num();
		k_Index = 0;
		while (index < nNodes)
		{
			currNode = index;
			while (k_Index < K)
			{
				if (currNode > nNodes)
				{
					break;
				}
				countWalks[currNode]++;
				g = rand() % 11;
				if (D * 10 < g && countEdge[currNode] > 0) 
				{
					nextNode = rand() % (countEdge[currNode]);
					currNode = graph[currNode][nextNode];
				}
				else 
				{
					currNode = rand() % nNodes;
				}
				k_Index++;
			}
			k_Index = 0;
			index += p;
		}
	}
	return 0;
}
/*
* Uses qsort in the end to determine most visited pages and prints the result 
*/
int sort(int nNodes, int p)
{
	int *listofNodes = (int*)malloc(sizeof(int)*nNodes);
	for (int i = 0; i < nNodes; i++)
	{
		listofNodes[i] = i;
	}
	qsort(listofNodes, nNodes, sizeof(int), comp);
	for (int i = 0; i < nNodes; i++)
	{
		printf("%d,%d\n", listofNodes[i], countWalks[listofNodes[i]]);
	}
	return 0;
}
int main(int argc, char *argv[])
{
	double D = 0;
	int K = 0;
	int nNodes = 0;
	int p;
	int** graph;
	srand(time(NULL));

	if (argc < 4)
	{
		printf("please run as [./program K D file.txt p]\n");
		return 0;
	}

	sscanf(argv[2], "%lf", &D);
	sscanf(argv[1], "%d", &K);
	sscanf(argv[4], "%d", &p);

	nNodes = getnNodes(argv);
	getmNodes(argv, nNodes);
	graph = initGraph(argv, nNodes);

	double timeCount = omp_get_wtime();

	walk(graph, nNodes, p, D, K);
	
	timeCount = omp_get_wtime() - timeCount;
	sort(nNodes, p);
	return 0;
}
