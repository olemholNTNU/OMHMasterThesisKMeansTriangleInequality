// C program that implements a Kmeans algorithm Lloyd version
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
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

int distance(Point a, Point b) {
	long distanceSum = 0;
	for(int i=0; i<amountOfCoord; i++) {
		long squared = abs(a.coord[i] - b.coord[i]);
		distanceSum += squared*squared;
	}

	return sqrt(distanceSum);
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

        int batchSize = 100000;

        if (argc >= 3) {
                char* tempChar;

                errno = 0;
                batchSize = strtol(argv[2], &tempChar, 10);
                if (*tempChar != '\0' || batchSize < 0 || batchSize >100000 || errno != 0) {
                        batchSize = 100000;
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

        Point * preMeanCenters;
        preMeanCenters = (Point*) malloc(sizeof(Point)*sizeCenters);
        if (preMeanCenters == NULL) {
                printf("Couldnt allocate centers\n");
                return 0;
        }

	Point newPoint;
	if (!(newPoint.coord = (long*) malloc(sizeof(long)))) {
		printf("Couldn't allocate space for newPoint");
		return 0;
	}
	int j = 0;

//	As this portion is intended to also handle continous input an idea might be to switch
//	 between two files as input streams, assuming the input streams are files.
//	Due to currently reading file needs to be updated to take the next set of input.
	for (int i=0; j<batchSize && !feof(inputData) && fscanf(inputData, "%ld", &newPoint.coord[i]);) {
		char c = fgetc(inputData);
		if (c == '\n') {
			j++;
//			printf("%d, %d\n", newPoint.coord[i-1], newPoint.coord[i]);
			i=0;

			// check correct implemention
			// Correct implementation should be: IF not enough centers, make the new point a center
			// else add it as a point, increase the size of the "points" array if at its limit
			if (currentAmountOfCenters >= MAXCENTERAMOUNT) {
//				points[currentAmountOfPoints++] = newPoint;
				points[currentAmountOfPoints].coord = (long*) malloc(sizeof(long)*amountOfCoord);
				for (int t=0; t<amountOfCoord;t++){
					points[currentAmountOfPoints].coord[t] = newPoint.coord[t];
				}
				newPoint.center = findClosestCenter(centers, newPoint);
				points[currentAmountOfPoints++].center = newPoint.center;
				// Delay incrementing amount of points by 1 so that next calculations dont need -1
				// in order to not iterate over the new point.

				centers[newPoint.center].center++;

				for (int o=0; o<amountOfCoord; o++) {
					preMeanCenters[newPoint.center].coord[o] += newPoint.coord[o];
					centers[newPoint.center].coord[o] = preMeanCenters[newPoint.center].coord[o]/centers[newPoint.center].center;
				}

				// plan for expanding points
				if (sizePoints <= j) {
					sizePoints += 1000;
					points = (Point*) realloc(points, sizeof(Point)*sizePoints);
				}

				// was planning a recalculation of which center the new point is closest to now
				// however, if the center has moved, it has moved in the direction of the point
				// and since that center already was the closest, a new calculations should
				// result in the same
//				newPoint.center = findClosestCenter(centers, newPoint);
//				points[currentAmountOfPoints++].center = newPoint.center;

			}
			else {
				// No complicated initialisation method, however, can be done with Lloyd as know the points beforehand.
				// MacQueen has harder time with alternative initialisation methods due to not knowning beforehand


				// commented line doesnt work as it assign NewPoint adress to the array I think :(
				// have to allocate new space
//				points[currentAmountOfPoints++] = newPoint;
				points[currentAmountOfPoints].coord = (long*) malloc(sizeof(long)*amountOfCoord);
				for (int t=0; t<amountOfCoord;t++){
					points[currentAmountOfPoints].coord[t] = newPoint.coord[t];
				}
				points[currentAmountOfPoints++].center = currentAmountOfCenters;

//				centers[currentAmountOfCenters++] = newPoint;
				centers[currentAmountOfCenters].coord = (long*) malloc(sizeof(long)*amountOfCoord);
				preMeanCenters[currentAmountOfCenters].coord = (long*) malloc(sizeof(long)*amountOfCoord);
				for (int t=0; t<amountOfCoord;t++){
					centers[currentAmountOfCenters].coord[t] = newPoint.coord[t];
					preMeanCenters[currentAmountOfCenters].coord[t] = newPoint.coord[t];
				}
				// used to keep track of how many points assigned
//				preMeanCenters[currentAmountOfCenters].center = 1; // should not be used unless a post findClosestCenter is run
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

	free(newPoint.coord); newPoint.coord = NULL;

        if (argc < 4 || strcmp(argv[3], "-acc")) {
                return 0;
        }

        char filenamebuffer[100];

        if (strcmp(argv[1], "---")) {
                sprintf(filenamebuffer, "%s%s", argv[1], "AnswerList.txt");
        }
        else {
                sprintf(filenamebuffer, "%s", "AnswerList.txt");
        }

        FILE * accuracy = fopen(filenamebuffer, "r");
        int accuracyexists = 1;
        if (!accuracy) {
                printf("No answerlist to compare accuracy, checked: %s\n", filenamebuffer);
                accuracyexists = 0;
                return 0;
        }
        int intbuffer;
        int totacc = 0;
        int *answersaccuracy = (int*) calloc(currentAmountOfCenters, sizeof(int));

        for (int b=0;b<currentAmountOfPoints;b++) {
                fscanf(accuracy, "%d", &intbuffer);
                if (points[b].center == intbuffer) {
                        answersaccuracy[points[b].center]++;
                        totacc++;
                }
        }
        if (currentAmountOfCenters > 1){
                printf("%d.. %d/%d", totacc, answersaccuracy[0], centers[0].center);
                for (int i = 1; i<currentAmountOfCenters; i++) {
                        printf(":%d/%d", answersaccuracy[i], centers[i].center);
                }
                printf("\n");
        }
        fclose(accuracy);

        printf("%s on %s\n", argv[0], (argc >= 2 && strcmp(argv[1], "---")) ? argv[1] : "processedTrainSettcp.txt");
        for (int q=0; q<currentAmountOfCenters; q++){
                printf(" %d:\n  {[ ", q);
                for (int r=0; r<amountOfCoord;r++){
                        printf("%ld, ", centers[q].coord[r]);
                }
                printf( "], %d} ", centers[q].center);
                if(accuracyexists == 1) { printf( "Acc: %f\n", (double)answersaccuracy[q]/(double)centers[q].center); }
        }
        if (accuracyexists == 1) { printf("Tot acc: %f\n", (double)totacc/(double)currentAmountOfPoints); }

        if (answersaccuracy) free(answersaccuracy);
        answersaccuracy = NULL;


	return 0;
}
