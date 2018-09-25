/*
 * Copyright (c) 2001, 2002, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Driver code for whale mating problem
 */
#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

#define NMATING 10

// CUSTOM VARIABLES
struct semaphore *sem_male;
struct semaphore *sem_female;
struct lock *hold;
volatile unsigned int male_counter;
volatile unsigned int female_counter;
volatile unsigned int matchmaker_counter;
struct cv *males;
struct cv *females;
struct cv *matchmakers;
volatile unsigned int mating;

static
void
male(void *p, unsigned long which)
{
	(void)p;
	kprintf("male whale #%ld starting\n", which);

	lock_acquire(hold);
	male_counter += 1;

	if (female_counter > 0 && matchmaker_counter > 0) {
	    male_counter -= 1;
	    cv_signal(males, hold);

	    matchmaker_counter -= 1;
	    cv_signal(matchmakers, hold);

	    male_counter -= 1;
    } else {
	    cv_wait(males, hold);
	}

	lock_release(hold);
    kprintf("male whale #%ld ending\n", which);

static
void
female(void *p, unsigned long which)
{
	(void)p;
	kprintf("female whale #%ld starting\n", which);

	lock_acquire(hold);
	female_counter += 1;

	if (male_counter > 0 && matchmaker_counter > 0) {

	    male_counter -= 1;
        cv_signal(males, hold);

        matchmaker_counter = 1;
        cv_signal(matchmakers, hold);

        female_counter -= 1;

	} else {
        cv_wait(females, hold);
    }


    lock_release(hold);
    kprintf("female whale #%ld ending\n", which);


}

static
void
matchmaker(void *p, unsigned long which)
{
	(void)p;
	kprintf("matchmaker whale #%ld starting\n", which);

	lock_acquire(hold);
	matchmaker_counter += 1;

	if (female_counter > 0 && male_counter > 0)
    //kprintf("matchmaker #%ld coordinating mating\n", which);

	    female_counter -= 1;
        cv_signal(females, hold);

        male_counter -= 1;
        cv_signal(males, hold);

        matchmaker_counter -= 1;

	} else {
        cv_wait(matchmakers, hold);
    }


	lock_release(hold);
    kprintf("matchmaker #%ld ending\n", which);
	// Implement this function
}


// Change this function as necessary
int
whalemating(int nargs, char **args)
{
	// INITIALIZING SEMAPHORES
	sem_male = sem_create("Male Whales", 0);
	sem_female = sem_create("Female Whales", 0);

	// INITIALIZE COUNTERS
	male_counter = 0;
	female_counter = 0;
	matchmaker_counter = 0;
	mating = 0;

	// INTIALIZE LOCK AND CV
	hold = lock_create("MATING LOCK");
	males = cv_create("MALES CV");
	females = cv_create("FEMALES CV");
	matchmakers = cv_create("MATCHMAKERS CV");


	int i, j, err=0;

	(void)nargs;
	(void)args;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < NMATING; j++) {
			switch(i) {
			    case 0:
				err = thread_fork("Male Whale Thread",
						  NULL, male, NULL, j);
				break;
			    case 1:
				err = thread_fork("Female Whale Thread",
						  NULL, female, NULL, j);
				break;
			    case 2:
				err = thread_fork("Matchmaker Whale Thread",
						  NULL, matchmaker, NULL, j);
				break;
			}
			if (err) {
				panic("whalemating: thread_fork failed: %s)\n",
				      strerror(err));
			}
		}
	}

	return 0;
}
