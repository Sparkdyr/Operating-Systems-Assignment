#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>

#define MAX_CHILDREN 24

static sem_t numFreeChildren;

void sigChild(int sigNum);

typedef double MathFunc_t(double);


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
double integrateTrap(MathFunc_t* func, double rangeStart, double rangeEnd, size_t numSteps)
{
	double rangeSize = rangeEnd - rangeStart;
	double dx = rangeSize / numSteps;

	double area = 0;
	for (size_t i = 0; i < numSteps; i++) {
		double smallx = rangeStart + i*dx;
		double bigx = rangeStart + (i+1)*dx;

		area += dx * ( func(smallx) + func(bigx) ) / 2; // Would be more efficient to multiply area by dx once at the end. 
	}

	return area;
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

void sigChild(int sigNum) {
	while (waitpid(-1, NULL, WNOHANG) > 0) {
		sem_post(&numFreeChildren);
	}
}

int main(void)
{
	struct sigaction sa_child = {
		.sa_handler = &sigChild,
		.sa_flags = SA_RESTART
	};

	pid_t childPid;
	double rangeStart;
	double rangeEnd;
	size_t numSteps;
	MathFunc_t* func;
	char funcName[10] = {'\0'};
	if (sem_init(&numFreeChildren, 0, MAX_CHILDREN) == -1) {
		perror("sem_init");
		_exit(1);
	}

	if (sigaction(SIGCHLD, &sa_child, NULL) == -1) {
		perror("sigaction");
		_exit(1);
	}
	printf("Query format: [func] [start] [end] [numSteps]\n");


	while (getValidInput(&func, funcName, &rangeStart, &rangeEnd, &numSteps)) {

		sem_wait(&numFreeChildren);
		childPid = fork();

		if (childPid < 0) {
			sem_post(&numFreeChildren);
			perror("fork");
			_exit(0);
		}
		else if (childPid == 0) { // child code
			double area = integrateTrap(func, rangeStart, rangeEnd, numSteps);

			printf("The integral of function \"%s\" in range %g to %g is %.10g\n", funcName, rangeStart, rangeEnd, area);
			_exit(0);
		}
		else {//parent code
			printf("Created child\n");
		}
		
	}
	printf("Waiting for all queries to finish before exiting\n");
	for (int i=0; i< MAX_CHILDREN; i++){
		sem_wait(&numFreeChildren);
	}
	_exit(0); // Forces more immediate exit than normal - **Use this to exit processes throughout the assignment!**
}
