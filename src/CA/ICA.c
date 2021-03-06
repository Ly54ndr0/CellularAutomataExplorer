#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include "ICA.h"
//#include "tools/countCluster.h"

#define DELTAQ_PRECISION 1000

typedef struct cell
{
	int state;
	float threshold;
} cell;

struct ICA
{
	cell *matrix;
	int L;
	float q;
	float avgState;
	float avgThres;
	unsigned int seed;
	int numberOfClusters;
} ICA = {NULL};

// STATIC FUNCTIONS DECLARATIONS

static int ICA_neighborSum(int x, int y);
static int ICA_getCellState(void *c);
//static void *ICA_getCellByPos(int row, int col, const matrix_t *m);
//static neighbor_t *ICA_getNeighbor(void *c, const matrix_t *m);
//static uint64_t ICA_getCellPos(void *c, const matrix_t *m);
static int ICA_fastCountClusters(void);
static int ICA_neighborSumMoore(int x, int y);
static int ICA_neighborSumVonNeumann(int x, int y);
static int ICA_neighborSumMoore(int x, int y);

// VALUE RETURNING FUNCTIONS !! not prototypes !!

int ICA_getL(void){return ICA.L;}
float ICA_getQ(void){return ICA.q;}
float ICA_getAvgState(void){return ICA.avgState;}
float ICA_getAvgThres(void){return ICA.avgThres;}
unsigned int ICA_getSeed(void){return ICA.seed;}
int ICA_getNumberOfClusters(void){return ICA.numberOfClusters;}

// MAIN FUNCTIONS

void ICA_new(int L, float q)
{
	cell *newMatrix;
	unsigned int newSeed = 0;
	float new_q;
	cell *c;

	printf("Creating new %s matrix...\n", ICA_TITLE);

	free(ICA.matrix);
	newMatrix = (cell *) calloc((L + 2) * (L + 2), sizeof(cell));

	if (newMatrix == NULL)
	{
		printf("Failed to create new %s with L=%d and q=%g\n", ICA_TITLE, L, q);
		while(getchar() != '\n');
	}
	else
	{
		newSeed = rand();
		srand(newSeed);

		for (int row = 1; row <= L; ++row)
			for (register int col = 1; col <= L; ++col)
			{ 
				c = (newMatrix + row * (L + 2) + col);
				c->state = rand() % 2 * 2 - 1;
				new_q = (rand() % DELTAQ_PRECISION * 2.0 / DELTAQ_PRECISION - 1.0) * q;
				c->threshold = new_q;
			}

		printf("Successfully created with seed=%u.\n\n",newSeed);
	}
	
	ICA.matrix = newMatrix;
	ICA.L = L;
	ICA.q = q;
	ICA.avgState = 0;
	ICA.avgThres = 0;
	ICA.numberOfClusters = 0;
	ICA.seed = newSeed;
}

void ICA_newSeeded(int L, float q, unsigned int newSeed)
{
	cell *newMatrix;
	float new_q;
	cell *c;

	printf("Creating new %s matrix with seed %u...\n", ICA_TITLE, newSeed);

	free(ICA.matrix);
	newMatrix = (cell *) calloc((L + 2) * (L + 2), sizeof(cell));

	if (newMatrix == NULL)
	{
		printf("Failed to create new %s with L=%d and q=%g\n", ICA_TITLE, L, q);
		while(getchar() != '\n');
	}
	else
	{
		srand(newSeed);

		for (int row = 1; row <= L; ++row)
			for (register int col = 1; col <= L; ++col)
			{ 
				c = (newMatrix + row * (L + 2) + col);
				c->state = rand() % 2 * 2 - 1;
				new_q = (rand() % DELTAQ_PRECISION * 2.0 / DELTAQ_PRECISION - 1.0) * q;
				c->threshold = new_q;
			}

		printf("Successfully created with seed=%u.\n\n",newSeed);
	}
	
	ICA.matrix = newMatrix;
	ICA.L = L;
	ICA.q = q;
	ICA.avgState = 0;
	ICA.avgThres = 0;
	ICA.numberOfClusters = 0;
	ICA.seed = newSeed;
}

void ICA_invertCell(int x, int y)
{
	cell * C = ICA.matrix + (y + 1) * (ICA.L + 2) + (x + 1);
	printf("Cell (%d, %d) inverted from ", x, y);
	printf("s=%d t=%g to ", C->state, C->threshold);
	C->state *= -1;
	C->threshold *= -1;
	printf("s=%d t=%g\n", C->state, C->threshold);
}

void ICA_setSeed(unsigned int newSeed)
{
	ICA.seed = newSeed;
	srand(newSeed);
	printf("New seed set to %u.\n", ICA.seed);
}

void ICA_delete(void)
{
	if (ICA.matrix != NULL)
		puts("ICA matrix deleted.");
	free(ICA.matrix);
	ICA.matrix = NULL;
}

void ICA_runMoore(int32_t cycles, int32_t steps)
{
	int x, y;
	float deltaQ;

	while (cycles > 0  || steps > 0)
	{
		for (;steps > 0; --steps)
		{
			x = rand() % ICA.L + 1;
			y = rand() % ICA.L + 1;

			deltaQ = rand() % DELTAQ_PRECISION * ICA.q / DELTAQ_PRECISION;

			cell *c = (ICA.matrix + y * (ICA.L + 2) + x);

			if (ICA_neighborSumMoore(x, y) > c->threshold)
			{
				c->state = 1;
				c->threshold += deltaQ;
			}
			else
			{
				c->state = -1;
				c->threshold -= deltaQ;
			}
		}
		if (cycles > 0 )
		{
			--cycles;
			steps = ICA.L * ICA.L;
		}
	}
}

void ICA_runVonNeumann(int32_t cycles, int32_t steps)
{
	int x, y;
	float deltaQ;

	while (cycles > 0  || steps > 0)
	{
		for (;steps > 0; --steps)
		{
			x = rand() % ICA.L + 1;
			y = rand() % ICA.L + 1;

			deltaQ = rand() % DELTAQ_PRECISION * ICA.q / DELTAQ_PRECISION;

			cell *c = (ICA.matrix + y * (ICA.L + 2) + x);

			if (ICA_neighborSumVonNeumann(x, y) > c->threshold)
			{
				c->state = 1;
				c->threshold += deltaQ;
			}
			else
			{
				c->state = -1;
				c->threshold -= deltaQ;
			}
		}
		if (cycles > 0 )
		{
			--cycles;
			steps = ICA.L * ICA.L;
		}
	}
}

// INFORMATION RETRIEVAL FUNCTIONS

void ICA_printState(void)
{
	putchar('\n');
	puts("# Save the lines below in a file with extension .pgm");
	printf("P2\n%d %d\n%d\n", ICA.L + 2, ICA.L + 2, 1);
	for (int row = 0; row < ICA.L + 2; ++row)
	{	
		for (int col = 0; col < ICA.L + 2; ++col)
		{
			int state = (ICA.matrix + row * (ICA.L + 2) + col)->state;
			putchar(state == 1 ? '1' : '0');
			putchar(' ');
		}
		putchar('\n');
	}		
}

void ICA_printThres(void)
{
	putchar('\n');
	puts("# Save the lines below in a file with extension .pgm");
	printf("P2\n%d %d\n%d\n", ICA.L + 2, ICA.L + 2, 255);
	for (int row = 0; row < ICA.L + 2; ++row)
	{
		for (int col = 0; col < ICA.L + 2; ++col)
		{
			float thres = (ICA.matrix + row * (ICA.L + 2) + col)->threshold;
			int intThres = (int) (255 * (thres + ICA.q)/(2 * ICA.q));
			intThres = intThres > 255 ? 255 : intThres;
			intThres = intThres < 0 ? 0 : intThres;
			printf("%-4d", intThres);
		}
		
		putchar('\n');
	}		
}

void ICA_updateStats(void)
{
	double totalState = 0;
	double totalThreshold = 0.0;

	cell *lastElement = ICA.matrix + (ICA.L + 2) * (ICA.L + 1);

	for (cell *i = ICA.matrix; i < lastElement; ++i)
	{
		totalState += i->state;
		totalThreshold += i->threshold;
	}
		
	ICA.avgState = totalState / (ICA.L * ICA.L);
	ICA.avgThres = totalThreshold / (ICA.L * ICA.L);
	/*matrix_t m = (matrix_t) 
	{
		(void *)ICA.matrix, 
		ICA.L, 
		ICA.L, 
		ICA_getCellState, 
		ICA_getCellByPos, 
		ICA_getNeighbor, 
		ICA_getCellPos
	};*/
	ICA.numberOfClusters = ICA_fastCountClusters();
}

static int ICA_neighborSumVonNeumann(int x, int y)
{
	int sum = 0;

	sum += (ICA.matrix + (y - 1) * ICA.L + (x))->state;
	sum += (ICA.matrix + (y + 1) * ICA.L + (x))->state;
	sum += (ICA.matrix + (y) * ICA.L + (x - 1))->state;
	sum += (ICA.matrix + (y) * ICA.L + (x + 1))->state;

	return sum;
}

static int ICA_neighborSumMoore(int x, int y)
{
	int sum = 0;

	sum += (ICA.matrix + (y - 1) * ICA.L + (x))->state;
	sum += (ICA.matrix + (y + 1) * ICA.L + (x))->state;
	sum += (ICA.matrix + (y) * ICA.L + (x - 1))->state;
	sum += (ICA.matrix + (y) * ICA.L + (x + 1))->state;

	sum += (ICA.matrix + (y - 1) * ICA.L + (x - 1))->state;
	sum += (ICA.matrix + (y - 1) * ICA.L + (x + 1))->state;
	sum += (ICA.matrix + (y + 1) * ICA.L + (x - 1))->state;
	sum += (ICA.matrix + (y + 1) * ICA.L + (x + 1))->state;

	return sum;
}

/*
// TOOLS FOR COUNTING CLUSTER

static int ICA_getCellState(void *c)
{
	return ((cell *) c)->state;
}


static void *ICA_getCellByPos(int row, int col, const matrix_t *m)
{
	cell *M = m->m;
	return (void *) (M + row * (m->rows + 2) + col);
}

static neighbor_t *ICA_getNeighbor(void *c, const matrix_t *m)
{
	static neighbor_t neighbor;
	cell *C = c;
	neighbor = (neighbor_t)
	{
		C - (m->cols + 2),
		C + (m->cols + 2),
		C + 1,
		C - 1
	};
	return &neighbor;
}

static uint64_t ICA_getCellPos(void *c, const matrix_t *m)
{
	return ((cell *) c) - ((cell *) m->m);
}
*/
static int ICA_fastCountClusters(void) 
{
	bool *cellLooked = calloc((ICA.L + 2) * (ICA.L + 2), sizeof(bool));
	int numberOfClusters = 0;
	
	cell *N, *S, *E, *W, *C;
	bool newCluster;
	int row, col;

	// queue
	int queueSize = 4 * ICA.L;
	void **queue = calloc(queueSize, sizeof(void *));
	int first = 0, last = 0, elements = 0;
	#define ENQUEUE(x) *(queue + last) = x; last = last + 1 < queueSize ? last + 1 : 0; ++elements
	#define DEQUEUE *(queue + first); first = first + 1 < queueSize ? first + 1 : 0; --elements

	if (queue == NULL || cellLooked == NULL)
	{
		puts("Failed to alloc in ICAcountCluster");
		while(getchar() != '\n');
	}

	// ALGORITHM
	for (row = 1; row <= ICA.L; ++row)
		for (col = 1; col <= ICA.L; ++col)
		{
			C = (ICA.matrix + row * (ICA.L + 2) + col);

			if (C->state != 1 || *(cellLooked + (C - ICA.matrix)) == true)
				continue;

			newCluster = false;
			*(cellLooked + (C - ICA.matrix)) = true;
			ENQUEUE(C);

			while (elements > 0)
			{
				C = DEQUEUE;

				N = C - ICA.L - 2;
				S = C + ICA.L + 2;
				E = C + 1;
				W = C - 1;
				
				if (N->state == 1 && *(cellLooked + (N - ICA.matrix)) == false)
				{
					*(cellLooked + (N - ICA.matrix)) = true;
					newCluster = true;
					ENQUEUE(N);
				} 
				if (S->state == 1 && *(cellLooked + (S - ICA.matrix)) == false)
				{
					*(cellLooked + (S - ICA.matrix)) = true;
					newCluster = true;
					ENQUEUE(S);
				} 
				if (E->state == 1 && *(cellLooked + (E - ICA.matrix)) == false)
				{
					*(cellLooked + (E - ICA.matrix)) = true;
					newCluster = true;
					ENQUEUE(E);
				} 
				if (W->state == 1 && *(cellLooked + (W - ICA.matrix)) == false)
				{
					*(cellLooked + (W - ICA.matrix)) = true;
					newCluster = true;
					ENQUEUE(W);
				} 
			}

			if (newCluster) ++numberOfClusters;
		}
		
	free(queue);
	free(cellLooked);

	return numberOfClusters;
}
