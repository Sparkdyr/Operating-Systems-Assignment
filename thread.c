#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#define NUM_THREADS 24

typedef double MathFunc_t(double);

typedef struct {
	double rangeStart;
	double rangeEnd;
	size_t numSteps;
	MathFunc_t* func;
	double* area;
	pthread_mutex_t* mutex;

} thread_vars;


double gaussian(double x)
{
	return exp(-(x*x)/2) / (sqrt(2 * M_PI));
}


double chargeDecay(double x)
{
	if (x < 0) {
		return 0;
	} else if (x < 1) {
		return 1 - exp(-5*x);
	} else {
		return exp(-(x-1));
	}
}





// Integrate using the trapezoid method. 
void* integrateTrap(void* arg)
{
	thread_vars* vars = arg;

	double rangeSize = vars->rangeEnd - vars->rangeStart;
	double dx = rangeSize / vars->numSteps;

	double area = 0;
	for (size_t i = 0; i < vars->numSteps; i++) {
		double smallx = vars->rangeStart + i*dx;
		double bigx = vars->rangeStart + (i+1)*dx;

		area += dx * ( vars->func(smallx) + vars->func(bigx) ) / 2; // Would be more efficient to multiply area by dx once at the end. 
	}

	pthread_mutex_lock(vars->mutex);
	*(vars->area) += area;
	pthread_mutex_unlock(vars->mutex);

	return NULL;
}




bool getValidInput(MathFunc_t** func, char* funcName, double* start, double* end, size_t* numSteps)
{
	memset(funcName, '\0', 10); // Clear funcName. Magic number used because format strings are annoying. 

	// Read input numbers and place them in the given addresses:
	size_t numRead = scanf("%9s %lf %lf %zu", funcName, start, end, numSteps);

	if (strcmp(funcName, "sin") == 0) {
		*func = &sin;
	} else if (strcmp(funcName, "gauss") == 0) {
		*func = &gaussian;
	} else if (strcmp(funcName, "decay") == 0) {
		*func = &chargeDecay;
	} else {
		*func = NULL;
	}

	// Return whether the given func and range is valid:
	return (numRead == 4 && *func != NULL && *end >= *start && *numSteps > 0);
}



int main(void)
{
	double rangeStart;
	double rangeEnd;
	size_t numSteps;
	MathFunc_t* func;
	char funcName[10] = {'\0'};
	
	pthread_t threads[NUM_THREADS];
	thread_vars vars[NUM_THREADS];
	double area = 0;
	pthread_mutex_t mutex;

	pthread_mutex_init(&mutex, NULL);

	printf("Query format: [func] [start] [end] [numSteps]\n");

	while (getValidInput(&func, funcName, &rangeStart, &rangeEnd, &numSteps)) {
		area = 0;
		
		size_t leftover = numSteps % NUM_THREADS;
		double rangeSize = (rangeEnd - rangeStart) / numSteps;
		size_t currentStep = 0;

		for (size_t i = 0; i < NUM_THREADS; i++) {
			size_t threadSteps = numSteps / NUM_THREADS;

			if (i < leftover) {
				threadSteps++;
			}

			vars[i].func = func;
			vars[i].rangeStart = rangeStart + currentStep * rangeSize;
			vars[i].rangeEnd = rangeStart + (currentStep + threadSteps) * rangeSize;
			vars[i].numSteps = threadSteps;
			vars[i].area = &area;
			vars[i].mutex = &mutex;

			pthread_create(&threads[i], NULL, integrateTrap, &vars[i]);

			currentStep += threadSteps;
		}

		for (size_t i = 0; i < NUM_THREADS; i++) {
			pthread_join(threads[i], NULL);
		}

		printf("The integral of function \"%s\" in range %g to %g is %.10g\n", funcName, rangeStart, rangeEnd, area);
	}

	pthread_mutex_destroy(&mutex);

	_exit(0); // Forces more immediate exit than normal - **Use this to exit processes throughout the assignment!**
}
