#include <stdio.h>
#include <math.h>
int
main(int argc, char *argv[])
{
        int view, nview;
        char filename[128];

        float mu1=0. , mu2=6., density, afac, s;

        FILE *fp = (FILE *)NULL;
        if (argc !=4){
                fprintf(stderr, "Usage: %s iview nview phmfilename\n", argv[0]);
                exit (1);
        }

        view = atoi(argv[1]);
        nview = atoi(argv[2]);
        sprintf(filename, "%s", argv[3]);

        s = (float)view/((float)(nview-1));

   if ( s < 7./16. )
                density = mu1;
   else if ( s  > 9./16. )
        density = mu2;
   else {
        afac = ( (s - 7./16.) / 2./16.);
        density = log(1/((1-afac)*exp(-mu1) + afac * exp(-mu2)));
   }

/*
        density = mu1 + (mu2-mu1)*s;
        if (s <=0.5)
                        density = mu1;
        else
                        density = mu2;
 */
        if ( (fp = fopen(filename, "w"))  == (FILE *)NULL){
                fprintf(stderr,"Error, can not open file \"tmpphmfile\"\n");
                exit(2);
        }
        fprintf(fp, "rectangle 0 0 11.5 11.5 0 0\n");
        fprintf(fp, "ellipse   0 0 11.4 11.4 0 1\n");
        fprintf(fp, "ellipse   0 0 1.25 1.25 0 %f\n", density);

        fclose(fp);
        exit(0);
}
