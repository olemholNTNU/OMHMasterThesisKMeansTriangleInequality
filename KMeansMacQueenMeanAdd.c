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
	double * coord;
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

double distance(Point a, Point b) {
	double distanceSum = 0;
	for(int i=0; i<amountOfCoord; i++) {
		double squared = abs(a.coord[i] - b.coord[i]);
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
		printf("%lf%s", a.coord[i], trailornot);
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
	double shortestDistanceCenter[2] = { distance(centers[newPoint.center], newPoint), newPoint.center };
	for(int i=0; i<currentAmountOfCenters; i++) {
		if (i == shortestDistanceCenter[1]) {
			continue;
		}
		double currDist = distance(centers[i], newPoint);
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

	Point * points;
	sizePoints = 1000;
	points = (Point*) malloc(sizeof(Point)*sizePoints);
	if (points == NULL) {
		printf("Couldnt allocate points\n");
		return 0;
	}

//	const int POINTSIZEINCREASEMODIFIER;

	Point * centers;
	sizeCenters = 2;
	centers = (Point*) malloc(sizeof(Point)*sizeCenters);
	if (centers == NULL) {
		printf("Couldnt allocate centers\n");
		return 0;
	}

	Point newPoint;
	if (!(newPoint.coord = (double*) malloc(sizeof(double)))) {
		printf("Couldn't allocate space for newPoint");
		return 0;
	}
	int j = 0;
	const int batchSize = 100000;

	long starttime = get_nanos();

//	As this portion is intended to also handle continous input an idea might be to switch
//	 between two files as input streams, assuming the input streams are files.
//	Due to currently reading file needs to be updated to take the next set of input.
	for (int i=0; j<batchSize && !feof(inputData) && fscanf(inputData, "%lf", &newPoint.coord[i]);) {
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
				points[currentAmountOfPoints].coord = (double*) malloc(sizeof(double)*amountOfCoord);
				for (int t=0; t<amountOfCoord;t++){
					points[currentAmountOfPoints].coord[t] = newPoint.coord[t];
				}
				newPoint.center = findClosestCenter(centers, newPoint);
				points[currentAmountOfPoints].center = newPoint.center;

				centers[newPoint.center].center++;
				// Delay incrementing amount of points by 1 so that next calculations dont need -1
				// in order to not iterate over the new point.

				// plan for expanding points
				if (sizePoints <= j) {
					sizePoints += 1000;
					points = (Point*) realloc(points, sizeof(Point)*sizePoints);
				}


				if (j < batchSize) {

					// change the center which had gotten a new point to have at least the coords of the newest point added
					for (int y=0; y<amountOfCoord; y++){
						centers[newPoint.center].coord[y] = (centers[newPoint.center].coord[y]*(centers[newPoint.center].center-1)/centers[newPoint.center].center)+(newPoint.coord[y]/centers[newPoint.center].center);
					}
//
//					// Identify all points, not including most recent addition, that belong to the center that point was added to
//					for (int m=0; m<currentAmountOfPoints;m++){
//						// skip on not correct point
//						if (points[m].center != newPoint.center) {
//							continue;
//						}
//						for (int o=0; o<amountOfCoord; o++) {
//							centers[newPoint.center].coord[o] += points[m].coord[o];
//						}
//					}
//
//					//		printf("3rd stage:\n");
//					// After finished counting/adding, then make the average point which is the new center.
//					for (int k=0; centers[newPoint.center].center>0 && k<amountOfCoord; k++) {
//						centers[newPoint.center].coord[k] = centers[newPoint.center].coord[k]/centers[newPoint.center].center;
//					}
				}

				currentAmountOfPoints++;
			}
			else {
				// No complicated initialisation method, however, can be done with Lloyd as know the points beforehand.
				// MacQueen has harder time with alternative initialisation methods due to not knowning beforehand


				// commented line doesnt work as it assign NewPoint adress to the array I think :(
				// have to allocate new space
//				points[currentAmountOfPoints++] = newPoint;
				points[currentAmountOfPoints].coord = (double*) malloc(sizeof(double)*amountOfCoord);
				for (int t=0; t<amountOfCoord;t++){
					points[currentAmountOfPoints].coord[t] = newPoint.coord[t];
				}
				points[currentAmountOfPoints++].center = currentAmountOfCenters;

//				centers[currentAmountOfCenters++] = newPoint;
				centers[currentAmountOfCenters].coord = (double*) malloc(sizeof(double)*amountOfCoord);
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
				newPoint.coord = realloc(newPoint.coord, sizeof(double)*amountOfCoord);
			}
			// could potentially have a check for reading an invalid number (i.e. a letter)
		}
	}

	fclose(inputData);

	// dont intend to use 'newPoint' anymore.
	free(newPoint.coord); newPoint.coord = NULL;
//	printPoints(centers, currentAmountOfCenters);
//	sleep(1);
//	printPoints(points, currentAmountOfPoints);
//	sleep(1);

//	printf("# of itterations: %d\n", itterations);
	// Alterantivly add a for loop that increases itterations and records results

	long finishtime = get_nanos();
	long runtime = finishtime - starttime;

        for (int x=0; x<currentAmountOfCenters; x++) {
                centers[x].center = 0;
        }
        for (int m=0; m<currentAmountOfPoints;m++){
                centers[points[m].center].center++;
        }


//	printPoints(centers, currentAmountOfCenters);

//	printPoints(centers, currentAmountOfCenters);
//	sleep(1);
//	printPoints(points, currentAmountOfPoints);
//	sleep(1);

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
//		printf("%d == %d ?\n", points[b].center, *intbuffer);
		if (points[b].center == *intbuffer) {
			answersaccuracy[points[b].center]++;
			totacc++;
//			printf("%d\n", totacc);
		}
	}
	if (currentAmountOfCenters > 1){
		printf("%d.. %d/%d:%d/%d\n",totacc, answersaccuracy[0], centers[0].center, answersaccuracy[1], centers[1].center);
	}
	free(intbuffer); intbuffer = NULL;
	fclose(accuracy);

	FILE * output = fopen("Resultstcp.txt", "a");

	fprintf(output,"%s on %s\n", argv[0], (argc >= 2 && strcmp(argv[1], "---")) ? argv[1] : "processedTrainSettcp.txt");
	for (int q=0; q<currentAmountOfCenters; q++){
		fprintf(output, " %d:\n  {[ ", q);
		for (int r=0; r<amountOfCoord;r++){
			fprintf(output, "%lf, ", centers[q].coord[r]);
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
