// C program that implements a Kmeans algorithm Lloyd version
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

int amountOfCoord = 1;

int currentAmountOfPoints = 0;
int currentAmountOfCenters = 0;

int sizePoints = 0;
int sizeCenters = 0;

int maxCenterAmount = 2;

struct Point {
	long * coord;
	int center;
} typedef Point;

long distance(Point a, Point b) {
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

	long itterations = 100;

	// Itterations # from run parameter
	if (argc >= 3) {
		char* tempChar;

		errno = 0;
		itterations = strtol(argv[2], &tempChar, 10);
		if (*tempChar != '\0' || itterations < 0 || itterations >100000 || errno != 0) {
			itterations = 100;
		}
	}

	int batchSize = 100000;

	if (argc >= 4) {
		char* tempChar;

		errno = 0;
		batchSize = strtol(argv[3], &tempChar, 10);
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
			i=0;

			// check correct implemention
			// Correct implementation should be: IF not enough centers, make the new point a center
			// else add it as a point, increase the size of the "points" array if at its limit
			if (currentAmountOfCenters >= maxCenterAmount) {
				points[currentAmountOfPoints].coord = (long*) malloc(sizeof(long)*amountOfCoord);
				for (int t=0; t<amountOfCoord;t++){
					points[currentAmountOfPoints].coord[t] = newPoint.coord[t];
				}
				points[currentAmountOfPoints++].center = findClosestCenter(centers, newPoint);
				// plan for expanding points
				if (sizePoints <= j) {
					sizePoints += 1000;
					points = (Point*) realloc(points, sizeof(Point)*sizePoints);
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
	if (newPoint.coord) free(newPoint.coord);
	newPoint.coord = NULL;
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

	if (argc < 5 || strcmp(argv[4], "-acc")) {
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


        printf("%s on %s\n# of Itt: %ld\n", argv[0], (argc >= 2 && strcmp(argv[1], "---")) ? argv[1] : "processedTrainSettcp.txt", itterations);
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
