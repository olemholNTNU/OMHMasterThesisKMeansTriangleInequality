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
int currentCenterDistances = 0;

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


// Elkans Modified
int findClosestCenter(Point centers[], double* centerdistances, double* lowerbounds, double* upperbound, Point newPoint) {
	// since we now remember what this point previously was, we dont calculate the distance anew.
//	long shortestDistanceCenter[2] = { distance(centers[newPoint.center], newPoint), newPoint.center };
	double shortestDistanceCenter = *upperbound;
	int whichCenter = newPoint.center;
	for(int i=0; i<currentAmountOfCenters; i++) {
		//Compare-Means Modification, but also Lemma 1 of Elkans method
		if (i == whichCenter || centerdistances[whichCenter*currentAmountOfCenters+i] >= 2*shortestDistanceCenter) {
			continue;
		}
//		printf("%ld vs %ld\n", centerdistances[shortestDistanceCenter[1]*currentAmountOfCenters+i], 2*shortestDistanceCenter[0]);
		double currDist = distance(centers[i], newPoint);
		lowerbounds[i] = currDist;
		if (currDist < shortestDistanceCenter) {
			shortestDistanceCenter = currDist;
			whichCenter = i;
		}
	}
	*upperbound = shortestDistanceCenter;
	return whichCenter;
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

//	const int POINTSIZEINCREASEMODIFIER;

	Point * centers;
	sizeCenters = 2;
	centers = (Point*) malloc(sizeof(Point)*sizeCenters);
	if (centers == NULL) {
		printf("Couldnt allocate centers\n");
		return 0;
	}


	double * centerDistances = (double*) malloc(sizeof(double));
	double * shortestCD = (double*) malloc(sizeof(double));

	double * lowerbounds = (double*) malloc(sizeof(double));
	double * upperbounds = (double*) malloc(sizeof(double));

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

				lowerbounds = (double*) realloc(lowerbounds, sizeof(double)*(currentAmountOfPoints+1)*currentAmountOfCenters);
				for (int e=0; e<currentAmountOfCenters; e++) {
					lowerbounds[currentAmountOfPoints*currentAmountOfCenters+e] = 0;
				}

				upperbounds = (double*) realloc(upperbounds, sizeof(double)*(currentAmountOfPoints+1));

				// assumes that closest center is the first center in centers
				newPoint.center = 0;
				upperbounds[currentAmountOfPoints] = distance(centers[newPoint.center], newPoint);
				lowerbounds[currentAmountOfPoints*currentAmountOfCenters+newPoint.center] = upperbounds[currentAmountOfPoints];

				// then use that assumption to find the actual closest center, checks all centers as at this point no known data for the point
				points[currentAmountOfPoints].center = findClosestCenter(centers, centerDistances, &lowerbounds[currentAmountOfPoints*currentAmountOfCenters], &upperbounds[currentAmountOfPoints], newPoint);

				currentAmountOfPoints++;

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

				centerDistances = realloc(centerDistances, sizeof(double)*currentAmountOfCenters*currentAmountOfCenters);
				shortestCD = realloc(shortestCD, sizeof(double)*currentAmountOfCenters);
				for (int v=0; v<currentAmountOfCenters; v++) {
					shortestCD[v] = -1;
					for (int w=0; w<currentAmountOfCenters; w++) {
						if (v != w) {
							centerDistances[v*currentAmountOfCenters+w] = distance(centers[v], centers[w]);
							if (shortestCD[v] < 0 || shortestCD[v] > centerDistances[v*currentAmountOfCenters+w]/2) {
								shortestCD[v] = centerDistances[v*currentAmountOfCenters+w]/2;
							}
						}
					}
				}


				lowerbounds = (double*) realloc(lowerbounds, sizeof(double)*currentAmountOfPoints*currentAmountOfCenters);
				for (int e=0; e<currentAmountOfCenters; e++) {
					lowerbounds[(currentAmountOfPoints-1)*currentAmountOfCenters+e] = 0;
				}

				upperbounds = (double*) realloc(upperbounds, sizeof(double)*currentAmountOfPoints);

				// since the added point is the zero distance from the center it also adds
				upperbounds[currentAmountOfPoints-1] = 0;
				lowerbounds[(currentAmountOfPoints-1)*currentAmountOfCenters+currentAmountOfCenters-1] = upperbounds[currentAmountOfPoints-1];
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

	int* upperVsShortCD = (int*) calloc(sizeof(int), currentAmountOfPoints);

	int* rArray = (int*) calloc(sizeof(int), currentAmountOfPoints);
	// let all of r(x) be false as upperbounds are not out of date

	Point* previousCenters = (Point*) malloc(sizeof(Point)*currentAmountOfCenters);
	for (int i=0; i<currentAmountOfCenters; i++) {
		previousCenters[i].coord = (double*) calloc(sizeof(double), amountOfCoord);
	}

	// do afterprocessing?
	for (int n=0; n<itterations; n++){

		// this is done in somewhat of a different order than compared to Elkans paper and instructions
		// this is because the read data in portion does parts and is not a correct representation
		//  of the points that belong to each center, therefore, centers are recalibrated at the start of the itteration

		// Zero all center, a centers center describes amount of points assigned to it.
		for (int x=0; x<currentAmountOfCenters; x++) {
			for (int y=0; y<amountOfCoord; y++){
				previousCenters[x].coord[y] = centers[x].coord[y];
				centers[x].coord[y] = 0;
			}
			previousCenters[x].center = centers[x].center;
			centers[x].center = 0;
		}

		// Find all Points currently assigned to each center, and add their coord to that center while counting
		for (int m=0; m<currentAmountOfPoints;m++){
			for (int o=0; o<amountOfCoord; o++) {
//				printf("%d: %d\n" ,o,points[m].center);
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

		for (int i=0; i<currentAmountOfPoints;i++) {
			for (int j=0; j<currentAmountOfCenters; j++) {
				int resultHolder = lowerbounds[i*currentAmountOfCenters+j] - distance(centers[j], previousCenters[j]);
				lowerbounds[i*currentAmountOfCenters+j] = (resultHolder > 0) ? resultHolder : 0;
			}
			upperbounds[i] = upperbounds[i] + distance(centers[points[i].center], previousCenters[points[i].center]);
			rArray[i] = 1;
		}

		for (int v=0; v<currentAmountOfCenters; v++) {
			shortestCD[v] = -1;
			for (int w=0; w<currentAmountOfCenters; w++) {
				if (v != w) {
					centerDistances[v*currentAmountOfCenters+w] = distance(centers[v], centers[w]);
					if (shortestCD[v] < 0 || shortestCD[v] > centerDistances[v*currentAmountOfCenters+w]/2) {
						shortestCD[v] = centerDistances[v*currentAmountOfCenters+w]/2;
					}
				}
			}
		}

//		printf("4th stage:\n");
		// Assign Points to new closest Center
		for (int p=0; p<currentAmountOfPoints; p++) {
			upperVsShortCD[p] = (upperbounds[p] <= shortestCD[points[p].center]) ? 1 : 0;

			if (!upperVsShortCD[p]) {
				for (int u=0; u<currentAmountOfCenters;u++) {
					if (points[p].center != u && upperbounds[p] > lowerbounds[p*currentAmountOfCenters+u] && upperbounds[p] > centerDistances[points[p].center*currentAmountOfCenters+u]/2 ) {
						if (rArray[p]) {
							upperbounds[p] = distance(points[p], centers[points[p].center]);
							lowerbounds[p*currentAmountOfCenters+points[p].center] = upperbounds[p];
							rArray[p] = 0;
						}
//						else {
//							distance p p.center = upperbounds[p]
//						}
						if (upperbounds[p] > lowerbounds[p*currentAmountOfCenters+u] || upperbounds[p] > centerDistances[points[p].center*currentAmountOfCenters+u]/2) {
							lowerbounds[p*currentAmountOfCenters+u] = distance(points[p], centers[u]);
							if (lowerbounds[p*currentAmountOfCenters+u] < upperbounds[p]) {
								centers[points[p].center].center--;
								points[p].center = u;
								centers[points[p].center].center++;
								upperbounds[p] = lowerbounds[p*currentAmountOfCenters+u];
							}
						}
					}
				}
			}

//			int newclosest = findClosestCenter(centers, centerDistances, points[p]);
//			if (newclosest !=points[p].center) {
//				centers[points[p].center].center--;
//				centers[newclosest].center++;
//			}
//			counter[points[p].center]++;
		}


	}

	long finishtime = get_nanos();
	long runtime = finishtime - starttime;

//	printPoints(centers, currentAmountOfCenters);

//	printPoints(centers, currentAmountOfCenters);
//	sleep(1);
//	printPoints(points, currentAmountOfPoints);
//	sleep(1);

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

//	fprintf(output,"KMeansElkan\n# of Itt: %ld\n", itterations);
        fprintf(output,"%s on %s\n# of Itt: %ld\n", argv[0], (argc >= 2 && strcmp(argv[1], "---")) ? argv[1] : "processedTrainSettcp.txt", itterations);
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
