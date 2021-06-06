/**********************************************************************************
 * Name: Chelsea Marie Hicks
 * 
 * Description: Month-by-month simulation of a grain growing operation that
 *      looks at the impacts of temperature, precipitation, and deer.
 *
 * Resources include:
 * Compiled on macOS using g++-10 -o grainsim grainsim.cpp -O3 -lm -fopenmp
***********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include <time.h>

int NowYear;                 // 2021-2026
int NowMonth;               // 0-11

float NowPrecip;            // inches of rain per month
float NowTemp;              // temperature this month
float NowHeight;            // grain height in inches
int NowNumDeer;             // number of deer in the current population
int NowNumHunters;          // number of hunters per year


//units of grain growth (inches), units of temp (degrees Fahrenheit),
//units of precipitation (inches)
const float GRAIN_GROWS_PER_MONTH =                 9.0;
const float ONE_DEER_EATS_PER_MONTH =               1.0;

const float AVG_PRECIP_PER_MONTH =                  7.0;    //average
const float AMP_PRECIP_PER_MONTH =                  6.0;    // plus or minus
const float RANDOM_PRECIP =                         2.0;    //plus or minus

const float AVG_TEMP =                              60.0;   //average
const float AMP_TEMP =                              20.0;   //plus or minus
const float RANDOM_TEMP =                           10.0;   //plus or minus

const float MIDTEMP =                               40.0;   
const float MIDPRECIP =                             10.0;

unsigned int seed = 0;

//function for squaring
float SQR(float x) {
    return x*x;
}

float Ranf(unsigned int *seedp, float low, float high) {
    float r = (float) rand_r(seedp);

    return (low + r * (high - low) / (float)RAND_MAX);
}

int Ranf(unsigned int *seedp, int ilow, int ihigh) {
    float low = (float) ilow;
    float high = (float) ihigh + 0.9999f;

    return (int) (Ranf(seedp, low, high));
}

float x = Ranf(&seed, -1.f, 1.f);

void Deer() {
    while(NowYear < 2027) {
        //compute temp next-val for this quantity based on
        //current state of the simulation

        int nextNumDeer = NowNumDeer;
        int carryingCapacity = (int) (NowHeight);

        if (nextNumDeer < carryingCapacity) {
            nextNumDeer++;
        }
        else if (nextNumDeer > carryingCapacity) {
            nextNumDeer--;
        }
        
        if (NowNumHunters > 0) {
            int deerKilled = NowNumHunters / 5;
            nextNumDeer -= deerKilled;
        }

        if (nextNumDeer <= 0) {
            nextNumDeer = 1;
        }

        //DoneComputing barrier:
        #pragma omp barrier
        NowNumDeer = nextNumDeer;

        //DoneAssigning barrier:
        #pragma omp barrier

        //DonePrinting barrier:
        #pragma omp barrier
    }
}

void Grain() {
    while(NowYear < 2027) {
        //compute temp next-val for this quantity based on
        //current state of the simulation

        float tempFactor = exp( -SQR( (NowTemp - MIDTEMP) / 10.));
        float precipFactor = exp( -SQR( (NowPrecip - MIDPRECIP) / 10.));

        float nextHeight = NowHeight;
        nextHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
        nextHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;

        if(nextHeight < 0.) {
            nextHeight = 0.;
        }

        //DoneComputing barrier:
        #pragma omp barrier

        NowHeight = nextHeight;

        //DoneAssigning barrier:
        #pragma omp barrier

        //DonePrinting barrier:
        #pragma omp barrier
    }
}

void Watcher() {
    while(NowYear < 2027) {
        //compute temp next-val for this quantity based on
        //current state of the simulation

        //DoneComputing barrier:
        #pragma omp barrier

        //DoneAssigning barrier:
        #pragma omp barrier

        //Print results
        printf("%d %d: %f degrees; %f cm precip; %d deer; %f cm height; %d hunters\n", NowMonth+1, NowYear, (5./9.) * (NowTemp-32.), NowPrecip * 2.54, NowNumDeer, NowHeight*2.54, NowNumHunters);

        //If at the end of the year, inc year and set NowMonth to 0, else inc NowMonth
        if (NowMonth == 11) {
            NowMonth = 0;
            NowYear++;
        }
        else {
            NowMonth++;
        }

        //Reset NowTemp
        float ang = (30. * (float) NowMonth + 15.) * (M_PI / 180.);
        float temp = AVG_TEMP - AMP_TEMP * cos(ang);
        NowTemp = temp + Ranf(&seed, -RANDOM_TEMP, RANDOM_TEMP);

        float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
        NowPrecip = precip + Ranf(&seed, -RANDOM_PRECIP, RANDOM_PRECIP);

        if (NowPrecip < 0.) {
            NowPrecip = 0.;
        }

        //DonePrinting barrier:
        #pragma omp barrier
    }
}

void MyAgent() {
    while(NowYear < 2027) {
        //compute temp next-val for this quantity based on
        //current state of the simulation

        int nextNumHunters = NowNumHunters;

        //Hunting season is Sept, Oct, Nov (8-10)
        if (NowMonth == 8) {
            nextNumHunters = 3;
        }
        else if (NowMonth == 9) {
            nextNumHunters = 4;
        }
        else if(NowMonth == 10) {
            nextNumHunters = 2;
        }
        else {
            nextNumHunters = 0;
        }

        //DoneComputing barrier:
        #pragma omp barrier
        NowNumHunters = nextNumHunters;

        //DoneAssigning barrier:
        #pragma omp barrier

        //DonePrinting barrier:
        #pragma omp barrier
    }
}

int main(int argc, char *argv[]) {
    //starting date and time
    NowMonth = 0;
    NowYear = 2021;

    //starting state
    NowNumDeer = 1;
    NowHeight = 3.;
    NowNumHunters = 0;

    omp_set_num_threads(4); //same as the number of sections
    #pragma omp parallel sections 
    {
        #pragma omp section
        {
            Deer();
        }

        #pragma omp section
        {
            Grain();
        }

        #pragma omp section
        {
            Watcher();
        }

        #pragma omp section
        {
            MyAgent();
        }

    } 
    //implied barrier -- all functions must return in order to allow
    //any of them to get past here

}