#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/************************************************************************************************************/
/*                       User Guide                                                                         */
/*         *** Everything to change is located right beneath this guide                                     */
/*  Data to input:                                                                                          */
/*      * fileName1 is the file with the project choices in. It can be produced by Excel.                   */
/*       Each column represents a pair of students, and each row a project. Values of 1 to 4 should be      */
/*           filled in, representing the pairs preference. If the pair have not picked a project,           */
/*           leave that cell empty.                                                                         */
/*           Make sure the file is a CSV file. There is a Excel error where leaving the top / bottom row    */
/*           empty can cause Excel to leave it out when saving the file as a CSV. This can be               */
/*           avoided with a dummy column of letters. See example file "Example of working around Excel error.csv"     */
/*                                                                                                          */
/*      * fileName2 is the file with the supervisor constraints in.                                         */
/*       Each project has a row, and each supervisor a column. For each project a supervisor                */
/*           supervises, a weighting is assigned. This should be between 0 and 1, and a decimal             */
/*           (not a fraction). Leave any other cells empty. See example file (include example).             */
/*           The weightings provide flexibility in the number of projects supervised by one lecturer but ensure that a */
/*           lecturer cannot be assigned more WORK than is appropriate.                                     */
/*           They are allowed up to (and including) a total of unit total weight or else the allocation is rejected. For example,  */
/*           a supervisor can be assigned two projects with weighting 0.5, but can't be assigned            */
/*           three projects with weighting 0.5. Alternatively a supervisor may be the main supervisor on one and three   */
/*           supervisor on two others another so one we can indicate this by weights 0.66, 0.33, 0.33 0.33. */
/*           Should be a CSV file.                                                                          */
/*                                                                                                          */
/* Variables to change:                                                                                     */
/*   * num_projects - this is the number of projects on offer (and hence the number of rows in both         */
/*            your fileName1 and fileName2.                                                                 */
/*   * num_groups - this is the number of pairs who have chosen projects (and hence the number of           */
/*            columns in fileName1). Update the constant "#define num_groups as well.                       */
/*   * num_supervisors - the number of supervisors (and hence the number of columns in fileName2).          */
/*   * weightings - 'weighti' is for preference i used in the function 'energy'. Needs to be integers.      */
/*                                                                                                          */
/* When compiling in the terminal, compile this program with the random num generator file 'ranvec.c'       */
/* and '-lm' for math library                                                                               */
/*                                                                                                          */
/*  Output files:                                                                                           */
/* The data saves to file 'finalConfig.txt' which contains the pair number, project number they are given   */
/* and their preference for this project.                                                                   */
/************************************************************************************************************/

/*Variables to change */
const int num_projects = 58;
const int num_groups   = 19;
const int num_supervisors = 27;
char fileName1[] = "StudentExample.csv"; /* This file has the data to fill choices - is passed into readChoices */
char fileName2[] = "SupervisorExample.csv"; /* This file has the data to fill in supConstraint - is passed into readLecturers */

/*weightings*/
/***THIS IS VERSION WITH 4.7, 4.15, 3, 2.3 (out of 5)**/
const float weight1 = (100.f / num_groups);
const float weight2 = (100.f / num_groups) * (4.15f / 4.7f);
const float weight3 = (100.f / num_groups) * (3.00f / 4.7f);
const float weight4 = (100.f / num_groups) * (2.35f / 4.7f);

float weights[5] =
{
    0,
    weight1,
    weight2,
    weight3,
    weight4
};

int group_for_project[num_projects] = {-1};

struct change_t
{
    int group;
    int old_project;
    int old_preference;
    int new_project;
    int new_preference;
}  /* optional variable list */;

/*global variables*/
double temp = 5; /* starting temperature */


float energy(int projPref[]); /* calculates energy of a given allocation */
int projClashFullCount(int projNum[]); /* counts clashes between allocations */
void generateRandomNumbers(); //ranvec.c
int randomNum(float random, int divisor); /* turns a random number into modulo divisor so we can use it */
void changeAllocationByPref(int choices[num_projects][num_groups], int projNum[num_groups], int projPref[num_groups], struct change_t *change); /* changes allocation of ONE PAIRS project based on random choice of preference */
void readChoices(int choices[num_projects][num_groups]); /* reads in the choices file */
void readLecturers(float supConstraint[num_projects][num_supervisors]); /* reads in the lecturer constraint file */
int countViolations(int projNum[], float supConstraint[num_projects][num_supervisors]); /* counts violations of constraints */
int countSupConstraintClashes(float supConstraint[num_projects][num_supervisors], int projNum[], int proj); /*counts violations of lectuere constraint */
int supervisor_has_clash(float supConstraint[num_projects][num_supervisors],
        int project);
void createInitialConfiguration(int choices[num_projects][num_groups], int projNum[num_groups], int projPref[num_groups], float supConstraint[num_projects][num_supervisors]); /* does what it says */
void cycleOfMoves(int choices[num_projects][num_groups], int projNum[num_groups], int projPref[num_groups], float supConstraint[num_projects][num_supervisors], FILE *saveData); /* Does all the moves for a fixed temp.*/
/* end of function initialisations */

int main()
{
    int choices[num_projects][num_groups]; /* This has the choices the pair made in. We import it from csv file. */
    float supConstraint[num_projects][num_supervisors]; /* This has all the data needed for calculating supervisor constraints in - including which projects a supervisor has and how many they can supervise. Imported from csv file */
    int i;
    int projNum[num_groups]; /* for each pair, stores what number project they are currently assigned */
    int projPref[num_groups]; /* for each pair, stores what preference their currently assigned project is. NOTE the preference stored here is not zero-indexed. */

    FILE *finalConfig; /* this file saves the final configuration - which pair have what project */
    FILE *saveData;

    generateRandomNumbers();

    /* read in Data */
    readChoices(choices);
    readLecturers(supConstraint);

    createInitialConfiguration(choices, projNum, projPref, supConstraint);
    /* We have a starting configuration WITH NO VIOLATIONS. */
    saveData = fopen("newData.txt", "w");

    //printf("Weight 1: %f\n", weight1);
    //printf("Weight 2: %f\n", weight2);
    //printf("Weight 3: %f\n", weight3);
    //printf("Weight 4: %f\n", weight4);

    /* Simulated Annealing time
       So, we stay at one temperature until either 1000*num_groups moves or 100*num_groups Succesful Moves.
       Then decrease and go again.
       */
    while(temp >= 0)
    {
        cycleOfMoves(choices, projNum, projPref, supConstraint, saveData);
        /* decrease temp */
        temp -= 0.001;
    }

    printf("Final energy is %f\n", energy(projPref));
    finalConfig = fopen("finalConfig.txt", "a");
    /* print final configuration to file */
    for(i = 0; i < num_groups; i++)
    {
        fprintf(finalConfig, "%d,%d,%d\n", i+1, projNum[i]+1, projPref[i]);
    }
    fprintf(finalConfig, "Final energy: %f\n", energy(projPref));
    fclose(finalConfig);
    fclose(saveData);

    return 0;
}

void cycleOfMoves(int choices[num_projects][num_groups], int projNum[num_groups], int projPref[num_groups], float supConstraint[num_projects][num_supervisors], FILE *saveData)
{
    int successfulmoves = 0;
    int moves = 0;
    float trialEnergy, currentEnergy;
    float changeEnergy;
    /*float ratioEnergy;*/
    int lecClashes;
    struct change_t change;

    double r;

    (void) saveData;

#define MAGIC_CALCULATION(p1, p2) \
    [(p1 - 1) << 2 | (p2 - 1)] = exp((weights[p2] - weights[p1]) / temp)
#define MAGIC_CALCULATION2(p1) \
    MAGIC_CALCULATION(p1,1), MAGIC_CALCULATION(p1,2), \
    MAGIC_CALCULATION(p1,3), MAGIC_CALCULATION(p1,4)

    float magic[] =
    {
        MAGIC_CALCULATION2(1), MAGIC_CALCULATION2(2),
        MAGIC_CALCULATION2(3), MAGIC_CALCULATION2(4)
    };

    generateRandomNumbers();
    currentEnergy = energy(projPref);
    /*printf("Temperature %f\nCurrent Energy = %f\n\n", temp,currentEnergy);*/
    while(moves < (1000 * num_groups) && successfulmoves < (100 * num_groups))
    {
        moves++;
        /* change the allocation here */
        changeAllocationByPref(choices, projNum, projPref, &change);

        changeEnergy = weights[change.old_preference] - weights[change.new_preference];
        trialEnergy = currentEnergy + changeEnergy;
        /*ratioEnergy = fabs(trialEnergy / currentEnergy);*/

        //printf("current energy and trial energy, %d, %d\n", currentEnergy, trialEnergy);

        /* reject due to lecturer constraint violation */
        lecClashes = supervisor_has_clash(supConstraint, change.new_project);
        if(lecClashes > 0)
            goto reject;

        /*
         *if(changeEnergy < 0)
         *    goto accept;
         */

        r = rand() / (double) RAND_MAX;
        /* Reject configuration due to energy - revert changes */
        /*if(temp > 0 && rands[0] > exp(-changeEnergy / temp))*/
        if(temp > 0 && r > magic[(change.old_preference - 1) << 2 | (change.new_preference - 1)])
            goto reject;
        /* Reject due to energy in T=0 case */
        else if(temp == 0 && changeEnergy > 0)
            goto reject;

        /* This is (in theory) impossible. We count because as its a nice
         * tracker for if things are broken. */
        if(changeEnergy == 0)
        {
            //printf("This shouldn't be happening?\n\n");
            continue;
        }

/*accept:*/
        successfulmoves++;
        currentEnergy = trialEnergy;
        //fprintf(saveData, "%d ", currentEnergy);
        //fprintf(saveData, "\n");
        continue;

reject:
        projNum[change.group] = change.old_project;
        projPref[change.group] = change.old_preference;
        group_for_project[change.old_project] = change.group;
        group_for_project[change.new_project] = -1;
    }
}


/* calculates energy of a given allocation. The weights are in here.
 * RETURNS energy */
float energy(int projPref[])
{
    int i = 0;
    float energy = 0;

    for(i = 0; i < num_groups; i++)
        energy -= weights[projPref[i]];

    return energy;
}

/* Counts how many clashes there are in the allocation.
 * RETURNS this. If 0, no clashes. */
int projClashFullCount(int projNum[])
{
    int i, j, count = 0;

    for(i = 0; i < num_groups; i++)
    {
        for(j = i + 1; j < num_groups; j++)
        {
            if(projNum[i] == projNum[j])
                count++;
        }
    }

    return count;
}

/* important number generator thingy */
void generateRandomNumbers()
{
    static int seed;
    seed++;

    /*seed = time(NULL);*/
    /*time((time_t *) &seed);*/
    srand(seed);
}

/* takes a random Number (which is between 0 and 1) and makes it between whatever we want. RETURNS this number. */
int randomNum(float random, int divisor)
{
    int number;
    random = random * 10000;
    number = (int) random % divisor;
    return number;
}

/* This functions CHANGES THE ALLOCATION. Based on picking a pair, and then picking a project,
 * and then making the change. Stores the change nicely in the changes function.*/
void changeAllocationByPref(int choices[num_projects][num_groups], int projNum[num_groups], int projPref[num_groups], struct change_t *change)
{
    double r;
    int pair, pref;
    int i;

    while(1)
    {
        r = rand() / (double) RAND_MAX;
        pair = randomNum(r, num_groups);
        //printf("\npair current pref is %d\n", projPref[pair]);

        /* Avoid picking same preference - waste of a move and time. */
        do
        {
            r = rand() / (double) RAND_MAX;
            pref = randomNum(r, 4) + 1;
        }
        while(projPref[pair] == pref);

        change->group = pair;
        change->old_project = projNum[pair];
        change->old_preference = projPref[pair];
        //printf("Energy before reallocation is %d\n", energy(projPref));
        /* make the change */
        for(i = 0; i < num_projects; i++)
        {
            if(choices[i][pair] == pref)
            {
                if(group_for_project[i] == -1)
                {
                    projNum[pair] = i;
                    projPref[pair] = pref;

                    change->new_project = i;
                    change->new_preference = pref;

                    group_for_project[i] = pair;
                    group_for_project[change->old_project] = -1;

                    return;
                }
                else
                    break;
            }
        }
        //printf("Energy after reallocation is %d\n", energy(projPref));
    }
}

/* Does what it says. RETURNS a count */
int countViolations(int projNum[], float supConstraint[num_projects][num_supervisors])
{
    int count = 0;
    int k;

    count += projClashFullCount(projNum);
    for (k = 0; k < num_groups; k++)
    {
        count += countSupConstraintClashes(supConstraint, projNum, projNum[k]);
    }

    return count;
}

/* counts how many times the lecturer/supervisor constraint is violated. */

//for each project assigned to a pair, how many times is lecturer constraint violated.
int countSupConstraintClashes(float supConstraint[num_projects][num_supervisors], int projNum[], int proj)
{
    int i, j, l;
    /*
       j is lectrer
       i is project / row
       l is pair.
       */
    float sum = 0;
    int clash = 0;

    /*so, for the project proj we look across the row to see which supervisors it has. Then we go down the supervisors column and sum up the energy of the projects allocated ONLY (projNum==i bit). If sum > 1, violation */
    for(j = 0; j < num_supervisors; j++)
    {
        sum = 0;
        if(supConstraint[proj][j] != 0)
        {
            for(i = 0; i < num_projects; i++)
            {
                for(l = 0; l < num_groups; l++)
                {
                    if(projNum[l] == i)
                    {
                        sum += supConstraint[i][j];
                    }
                }
            }

            if (sum > 1)
                clash++;
        }
    }

    return clash;
}

int supervisor_has_clash(float supConstraint[num_projects][num_supervisors],
        int project)
{
    int supervisor;
    int proj;

    float sum;

    for(supervisor = 0; supervisor < num_supervisors; supervisor++)
    {
        if(supConstraint[project][supervisor] != 0)
        {
            sum = 0;
            for(proj = 0; proj < num_projects; proj++)
            {
                if(supConstraint[proj][supervisor] != 0 &&
                   group_for_project[proj] != -1)
                {
                    sum += supConstraint[proj][supervisor];
                }
            }

            if(sum > 1)
                return 1;
        }
    }

    return 0;
}

/* create an initial configuration. Start at random, and then accept any change (again randomly determined) that reduces the number of constraints being violated. We are finished when no constraints are being vioalted. */
void createInitialConfiguration(int choices[num_projects][num_groups], int projNum[num_groups], int projPref[num_groups], float supConstraint[num_projects][num_supervisors])
{
    int violationCount1, violationCount2; /* count number of violations. 1 is "old", 2 is "current" */
    int pref; /* integer from 1 to 4 */
    int i, j;
    struct change_t change;

    for (i = 0; i < num_groups; i++)
    {
        double r = rand() / (double) RAND_MAX;
        pref = randomNum(r, 4) + 1;

        /* find the choice with the preference, and assign it */
        for(j = 0; j < num_projects; j++)
        {
            if(choices[j][i] == pref)
            {
                projNum[i] = j;
                projPref[i] = pref;
            }
        }
    }

    /* Technically, we should fill this with true information, but we need
     * it to be filled with -1 so that we can get an initial configuration
     * the real information will be filled AFTER said initial configuration
     * has been achieved */
    for(i = 0; i < num_projects; i++)
        group_for_project[i] = -1;

    violationCount1 = countViolations(projNum, supConstraint);
    while(violationCount1 > 0)
    {
        //  printf("violationCount1=%i\n",violationCount1);

        changeAllocationByPref(choices, projNum, projPref, &change);
        violationCount2 = countViolations(projNum, supConstraint);

        /* In this case, the number of violations has INCREASED, so we
         * REJECT it and REVERT to the old allocation. */
        if(violationCount2 > violationCount1)
        {
            projNum[change.group] = change.old_project;
            projPref[change.group] = change.old_preference;

            group_for_project[change.old_project] = change.group;
            group_for_project[change.new_project] = -1;
        }
        /* update violationCount1 */
        else
        {
            violationCount1 = violationCount2;
        }
    }

    for(i = 0; i < num_groups; i++)
        group_for_project[projNum[i]] = i;
}

/* read in the data for which pairs have what projects as their choices */
void readChoices(int choices[num_projects][num_groups])
{
    int c;
    FILE *fin;
    fin = fopen(fileName1, "r");

    unsigned int row = 0;
    unsigned int column = 0;

    // Get the first character
    c = fgetc(fin);

    while(c != EOF)
    {
        if(c == '\r')
            c = fgetc(fin);

        if(c == '\n')
        {
            row++;
            column = 0;
        }

        if(c == ',')
        {
            choices[row][column] = 0;
            column++;

            // If the next character is a newline (or carriage return)
            // we have to set the last column to a 0 because we can't
            // detect it otherwise
            c = fgetc(fin);
            if(c == '\r' || c == '\n')
                choices[row][column] = 0;
            continue;
        }

        if(c != ',' && c != '\n' && c != '\r')
        {
            int pref;
            ungetc(c, fin);
            fscanf(fin, "%i", &pref);
            choices[row][column] = pref;
            column++;

            // If the next character after the number is a comma we have
            // to consume it to avoid counting it as if it were an
            // empty column
            c = fgetc(fin);
            if(c != ',')
                ungetc(c, fin);
        }

        c = fgetc(fin);
    }

    fclose(fin);
}

/* reads in the lecturer constraint into supConstraint */
void readLecturers(float supConstraint[num_projects][num_supervisors])
{
    char c;
    FILE *fin;
    fin = fopen(fileName2, "r");

    unsigned int row = 0;
    unsigned int column = 0;

    // Get the first character
    c = fgetc(fin);

    while(c != EOF)
    {
        if(c == '\r')
            c = fgetc(fin);

        if(c == '\n')
        {
            row++;
            column = 0;
        }

        if(c == ',')
        {
            supConstraint[row][column] = 0;
            column++;

            // If the next character is a newline (or carriage return)
            // we have to set the last column to a 0 because we can't
            // detect it otherwise
            c = fgetc(fin);
            if(c == '\r' || c == '\n')
                supConstraint[row][column] = 0;
            continue;
        }

        if(c != ',' && c != '\n' && c != '\r')
        {
            ungetc(c, fin);
            fscanf(fin, "%f", &supConstraint[row][column]);
            column++;

            // If the next character after the number is a comma we have
            // to consume it to avoid counting it as if it were an
            // empty column
            c = fgetc(fin);
            if(c != ',')
                ungetc(c, fin);
        }

        c = fgetc(fin);
    }

    fclose(fin);
}
