//
//  main.c
//  Lab1
//
//  Created by Fahad Mahmood on 2019-01-10.
//  Copyright © 2019 Fahad Mahmood. All rights reserved.
//

# include <stdio.h>
# include <signal.h>
# include <unistd.h>
# include <stdlib.h>

// user - defined signal handler for alarm .
void alarm_handler ( int signo ) {
    if (signo == SIGALRM) {
        printf ("Alarm\n") ;
    }
}

void Ctrl_C ( int signo ) {
    if (signo == SIGINT) {
        printf ("Ctrl+C pressed\n") ;
    }
}

void Ctrl_Z ( int signo ) {
    if (signo == SIGTSTP) {
        printf ("Ctrl+Z was pressed\n") ;
        exit(1);
    }
}

int main ( void ) {
    // register the signal handler
    if (signal ( SIGALRM , alarm_handler ) == SIG_ERR ) {
        printf ("failed to register alarm handler.") ;
        exit (1) ;
    }
    
    if (signal ( SIGINT , Ctrl_C ) == SIG_ERR ) {
        printf ("failed to register.") ;
        exit (1) ;
    }
    
    if (signal ( SIGTSTP , Ctrl_Z ) == SIG_ERR ) {
        printf ("the program was stopped.") ;
        exit (1) ;
    }
    
    //alarm (3) ; // set alarm to fire in 3 seconds .
    while (1) {
        sleep (5) ;
        alarm (2) ;
    } // wait until alarm goes off
}
