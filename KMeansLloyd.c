// C program that implements a Kmeans algorithm Lloyd version
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>

int amountOfCoord = 1;

int currentAmountOfPoints = 0;
int currentAmountOfCenters = 0;

int sizePoints = 0;
int sizeCenters = 0;

const int MAXCENTERAMOUNT = 2;

// Whether the variables should be ints or floats is undecided, coords should probably be floats
struct Point {
	long * coord;
	int center;
} typedef Point;

// Taken from: https://stackoverflow.com/questions/361363/how-to-measure-time-in-milliseconds-using-ansi-c/36095407#36095407
// This seems to create Segfaults, though also experienced segfaultss with the contents of function being empty so not definitive
// Not sure of the source of the segfaults, but it is not consistent, and not intending to check every address access for not NULL
//  most array accesses should already be checked. Issue seems resolved, related to something else instead (bad handling of pointer).
static long get_nanos(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

long distance(Point a, Point b) {
	long distanceSum = 0;
	for(int i=0; i<amountOfCoord; i++) {
		long squared = abs(a.coord[i] - b.coord[i]);
		distanceSum += squared*squared;
	}

	/* INPUT SQUARE ROOT FUNCTIONALITY IF NEEDED */
	// Suspected to modify the value of lower valued dimensions (increasing value of lower distances relatively)

	// a inaccurate method of square root is to get the average of each coord,
	//  then divide the distanceSum by that average, getting an apporximate which for the same process can be done
	// will be inaccurate most of the time, however, likely a non-consequential inaccuracy if repeated enough.

	return sqrt(distanceSum);
}

void printPoint(Point a) {
	char trail[] = ", ";
	char ornot[] = " ";
	char *trailornot = trail;

	printf("{[ ");
	for (int i=0; i<amountOfCoord; i++) {
		if (i == amountOfCoord-1){
			trailornot = ornot;
		}
		printf("%ld%s", a.coord[i], trailornot);
	}
	printf("], %d }\n", a.center);

}

void printPoints(Point a[], int amount) {
	for (int i=0; i<amount; i++) {
		printf("%d: ", i);
		printPoint(a[i]);
	}
}

void addPoint(Point pointList[], Point toAdd, int * counter, int maxamount) {
        if (*counter < maxamount) {
                pointList[*counter] = toAdd;
		++(*counter);
        }
        else {
                printf("Amount of points are full, need to expand array to fit more\n");
        }
}

// Elkan version should also ask for the center inbetween distances to eliminate uneeded calculations
int findClosestCenter(Point centers[], Point newPoint) {
	long shortestDistanceCenter[2] = { distance(centers[newPoint.center], newPoint), newPoint.center };
	for(int i=0; i<currentAmountOfCenters; i++) {
		if (i == shortestDistanceCenter[1]) {
			continue;
		}
		long currDist = distance(centers[i], newPoint);
		if (currDist < shortestDistanceCenter[0]) {
			shortestDistanceCenter[0] = currDist;
			shortestDistanceCenter[1] = i;
		}
	}
	return shortestDistanceCenter[1];
}

int main(int argc, char *argv[]) {

	FILE *inputData;
	if ( argc >= 2 && sizeof(argv[1]) <= 50*sizeof(char) && strcmp(argv[1], "---")) {
		printf("Reading from file: %s\n", argv[1]);
		inputData = fopen(argv[1], "r");
	}
	else {
		printf("Reading from file: processedTrainSettcp.txt\n");
		inputData = fopen("processedTrainSettcp.txt", "r");
	}

	if(!inputData) {
		printf("\nCouldnt find Data File.\n");
		return 0;
	}

	long itterations = 100;

	// Itterations # from run parameter
	if (argc == 3) {
		// Some seg faults happenes, unknown reason, however, happens less with this than
		//  previously used code... Think source of Segfaults was bad argv[1] into open file
		//  using a pointer to identify filename between argv or a static name. Seems fixed
		char* tempChar;

		// used some of this to handle conversion/errors: https://stackoverflow.com/a/9748431
		errno = 0;
		itterations = strtol(argv[2], &tempChar, 10);
		if (*tempChar != '\0' || itterations < 0 || itterations >100000 || errno != 0) {
			itterations = 100;
		}
	}

	Point * points;
	sizePoints = 1000;
	points = (Point*) malloc(sizeof(Point)*sizePoints);
	if (points == NULL) {
		printf("Couldnt allocate points\n");
		return 0;
	}

	Point * centers;
	sizeCenters = 2;
	centers = (Point*) malloc(sizeof(Point)*sizeCenters);
	if (centers == NULL) {
		printf("Couldnt allocate centers\n");
		return 0;
	}

	Point newPoint;
	if (!(newPoint.coord = (long*) malloc(sizeof(long)))) {
		printf("Couldn't allocate space for newPoint");
		return 0;
	}
	int j = 0;
	const int batchSize = 100000;

	long starttime = get_nanos();

//	As this portion is intended to also handle continous input an idea might be to switch
//	 between two files as input streams, assuming the input streams are files.
//	Due to currently reading file needs to be updated to take the next set of input.
	for (int i=0; j<batchSize && !feof(inputData) && fscanf(inputData, "%ld", &newPoint.coord[i]);) {
		char c = fgetc(inputData);
		if (c == '\n') {
			j++;
			i=0;

			// check correct implemention
			// Correct implementation should be: IF not enough centers, make the new point a center
			// else add it as a point, increase the size of the "points" array if at its limit
			if (currentAmountOfCenters >= MAXCENTERAMOUNT) {
				points[currentAmountOfPoints].coord = (long*) malloc(sizeof(long)*amountOfCoord);
				for (int t=0; t<amountOfCoord;t++){
					points[currentAmountOfPoints].coord[t] = newPoint.coord[t];
				}
				points[currentAmountOfPoints++].center = findClosestCenter(centers, newPoint);
				// plan for expanding points
				if (sizePoints <= j) {
					sizePoints += 1000;
					points = (Point*) realloc(points, sizeof(Point)*sizePoints);

					// This section can be modified to exit the loop instead
					//  (or add a condition in the for itself)
					// However, the batchsize exists to do that functionality
				}
			}
			else {
				// No complicated initialisation method, however, can be done with Lloyd as know the points beforehand.
				// MacQueen has harder time with alternative initialisation methods due to not knowning beforehand


				// commented line doesnt work as it assign NewPoint adress to the array I think :(
				// have to allocate new space
				points[currentAmountOfPoints].coord = (long*) malloc(sizeof(long)*amountOfCoord);
				for (int t=0; t<amountOfCoord;t++){
					points[currentAmountOfPoints].coord[t] = newPoint.coord[t];
				}
				points[currentAmountOfPoints++].center = currentAmountOfCenters;

				centers[currentAmountOfCenters].coord = (long*) malloc(sizeof(long)*amountOfCoord);
				for (int t=0; t<amountOfCoord;t++){
					centers[currentAmountOfCenters].coord[t] = newPoint.coord[t];
				}
				// used to keep track of how many points assigned
				centers[currentAmountOfCenters++].center = 1;
			}
		}
		else {
			ungetc(c, inputData);
			i++;
			// Should only do this in the first iteration, hence j == 0.
			if (j == 0) {
				amountOfCoord++;
				newPoint.coord = realloc(newPoint.coord, sizeof(long)*amountOfCoord);
			}
			// could potentially have a check for reading an invalid number (i.e. a letter)
		}
	}

	fclose(inputData);

	// dont intend to use 'newPoint' anymore.
	free(newPoint.coord); newPoint.coord = NULL;
	// Alterantivly add a for loop that increases itterations and records results

	// do afterprocessing?
	for (int n=0; n<itterations; n++){
		// Zero all center, a centers center describes amount of points assigned to it.
		for (int x=0; x<currentAmountOfCenters; x++) {
			for (int y=0; y<amountOfCoord; y++){
				centers[x].coord[y] = 0;
			}
			centers[x].center = 0;
		}

		// Find all Points currently assigned to each center, and add their coord to that center while counting
		for (int m=0; m<currentAmountOfPoints;m++){
			for (int o=0; o<amountOfCoord; o++) {
				centers[points[m].center].coord[o] += points[m].coord[o];
			}
			centers[points[m].center].center++;
		}

		// After finished counting/adding, then make the average point which is the new center.
		for (int l=0; l<currentAmountOfCenters; l++) {
			for (int k=0; centers[l].center>0 && k<amountOfCoord; k++) {
				centers[l].coord[k] = centers[l].coord[k]/centers[l].center;
			}
		}
		// Assign Points to new closest Center
		for (int p=0; p<currentAmountOfPoints; p++) {
			int newclosest = findClosestCenter(centers, points[p]);
			if (newclosest !=points[p].center) {
				centers[points[p].center].center--;
				centers[newclosest].center++;
			}
			points[p].center = newclosest;
		}
	}

	long finishtime = get_nanos();
	long runtime = finishtime - starttime;

	for (int x=0; x<currentAmountOfCenters; x++) {
		centers[x].center = 0;
	}
	for (int m=0; m<currentAmountOfPoints;m++){
		centers[points[m].center].center++;
	}

	FILE * accuracy = fopen("AnswerListtcp.txt", "r");
	int accuracyexists = 1;
	if (!accuracy) {
		printf("No answerlist to compare accuracy\n");
		accuracyexists = 0;
	}
	int *intbuffer = (int*)malloc(sizeof(int));
	int totacc = 0;
	int *answersaccuracy = (int*) calloc(currentAmountOfCenters, sizeof(int));

	for (int b=0;b<currentAmountOfPoints;b++) {
		fscanf(accuracy, "%d", intbuffer);
		if (points[b].center == *intbuffer) {
			answersaccuracy[points[b].center]++;
			totacc++;
		}
	}
	if (currentAmountOfCenters > 1){
		printf("%d.. %d/%d:%d/%d\n",totacc, answersaccuracy[0], centers[0].center, answersaccuracy[1], centers[1].center);
	}
	free(intbuffer); intbuffer = NULL;
	fclose(accuracy);

	FILE * output = fopen("Resultstcp.txt", "a");

        fprintf(output,"%s on %s\n# of Itt: %ld\n", argv[0], (argc >= 2 && strcmp(argv[1], "---")) ? argv[1] : "processedTrainSettcp.txt", itterations);
	for (int q=0; q<currentAmountOfCenters; q++){
		fprintf(output, " %d:\n  {[ ", q);
		for (int r=0; r<amountOfCoord;r++){
			fprintf(output, "%ld, ", centers[q].coord[r]);
		}
		fprintf(output, "], %d} ", centers[q].center);
		if(accuracyexists == 1) { fprintf(output, "Acc: %f\n", (double)answersaccuracy[q]/(double)centers[q].center); }
	}
	if (accuracyexists == 1) { fprintf(output, "Tot acc: %f\n", (double)totacc/(double)currentAmountOfPoints); }
	fprintf(output, "Time: %ld nanosec\n", runtime);

	fclose(output);
	free(answersaccuracy); answersaccuracy = NULL;

	return 0;
}
