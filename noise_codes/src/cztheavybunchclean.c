/* CZTHEAVYBUNCHCLEAN
 * Author	: Ajay Ratheesh, TIFR Mumbai
			: Mayuri Shinde, IUCAA Pune
			: Ajay Vibhute, IUCAA Pune
 * Date 	: 14-08-2017
 * 
 * Updated by Ajay Ratheesh on 20th Dec 2017
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fitsio.h"

#define bun_size_thresh 15
#define heavy_bunch_time_threshold_LLD 0.2
#define heavy_bunch_time_threshold_all 0.005
#define module_lld_threshold 10.0
#define bunch_time_threshold_small 0.001

void processLLDHeavyBunchClean(char *evtfile,char *bunchfile,char *lldThresholdFile);
void printerror( int status);
void createEventFile(char *outputfile,char *evtfile);
void writeEvent(double *evttime,double *cztseccnt,unsigned short *cztntick,unsigned short *pha,unsigned char *detid,unsigned char *pixid,unsigned char*detx,unsigned char *dety,unsigned short *veto,unsigned char *alpha,int *pi,float *energy,char *outputfile,int bufsize,int hdunum);
void modifyEventHeaderParams(char *outputfile);
void modifyExposure(char *outputfile, double **pix_exposure,int qid);



int main(int argc, char **argv)
{
	if(argc==4)
        {
		processLLDHeavyBunchClean(argv[1],argv[2],argv[3]);
		printf("\n Heavy Bunch Clean COMPLETED SUCCESSFULLY.\n");
	}
	if(argc<4)
	{
		printf("Enter all command line arguments\n1:Event file name\n2.Bunch file name\n3.CALDB lld file\n");
		exit(-1);
	}
	
	return 0;
}

void processLLDHeavyBunchClean(char *evtfile,char *bunchfile,char *lldThresholdFile)
{
	fitsfile *caldbfptr,*evtfptr,*bunchfptr;  
	FILE *logfile;     
	double *buntime_heavy,*buntime_real;
	int *bunsize_real;
	unsigned char *buntime_dfs,*buntime_dsl;
	int ii,flag,status, hdunum, hdutype,  nfound, anynull,i,intnull,llddetidcolnum,lldpixidcolnum,lldcolnum,qid;
	long frow, felem, nelem,nrows,evtnrows,bunchnrows, longnull,size=1;
	double *bunchtime;
	int j,*event_flag,*bunch_flag;
	unsigned char *numevent,*bunch_detid1,*bunch_detid2,*bunch_detid3,*bunch_detid4,*bunch_detid;
	unsigned char *caldb_detid,*caldb_pixid,*pix_flag,*final_pix_flag,bytenull,*caldb_detx,*caldb_dety;
	double *evttime,*cztseccnt,*finalevttime,*finalcztseccnt,doublenull;
        unsigned short *cztntick,*veto,*finalcztntick,*finalveto,*pha,*finalpha;
	unsigned char *detid,*pixid,*detx,*dety,*alpha,*finaldetid,*finalpixid,*finaldetx,*finaldety,*finalalpha;
	float *energy,floatnull,*finalenergy;
        int *pi,*finalpi;
        int lld_2d[64][64],ignore_count[2],*caldb_lld;
	int energycolnum,picolnum,timecolnum,cztseccntcolnum,cztntickcolnum,phacolnum,detidcolnum,pixidcolnum,detxcolnum,detycolnum,vetocolnum,alphacolnum,bunchtimecolnum,numeventcolnum,bunchdetidcolnum;
	int quadstart=0,quadend=3;
	int **dph;
	float *sd,*mean;
	char outevtfile[1000],outbadpixfile[1000],outlogfile[1000];
	int MODULE_LT_THRESHOLD=50,MODULE_GT_THRESHOLD=100,LT=-1,GT=-2;
	int HEAVY_BUNCH_THRESHOLD=10,HEAVIER_BUNCH_THRESHOLD=15,HEAVIER=-3,HEAVY=-4;

	int bunch_det_count[16];
	int *bunsize;
	//unsigned char *bunsize;
	status = 0;
	hdunum = 2;

		caldb_detid  = (unsigned char*)malloc(size * sizeof(unsigned char));
		caldb_pixid  = (unsigned char*)malloc(size * sizeof(unsigned char));
		caldb_detx  = (unsigned char*)malloc(size * sizeof(unsigned char));
		caldb_dety  = (unsigned char*)malloc(size * sizeof(unsigned char));
		caldb_lld  = (int*)malloc(size * sizeof(int));
  		
		bunchtime  = (double*)malloc(size * sizeof(double));
		bunsize  = (int*)malloc(size * sizeof(int));
		bunch_detid1  = (unsigned char*)malloc(size * sizeof(unsigned char));
		bunch_detid2  = (unsigned char*)malloc(size * sizeof(unsigned char));
		bunch_detid3  = (unsigned char*)malloc(size * sizeof(unsigned char));
		bunch_detid4  = (unsigned char*)malloc(size * sizeof(unsigned char));
		bunch_detid  = (unsigned char*)malloc(size * sizeof(unsigned char));
		buntime_real =(double*)malloc(sizeof(double)*size);
		buntime_dfs=(unsigned char*)malloc(sizeof(unsigned char)*size);
		buntime_dsl=(unsigned char*)malloc(sizeof(unsigned char)*size);
		bunsize_real =(int*)malloc(sizeof(int)*size);
		
		
		
		evttime  = (double*)malloc(size * sizeof(double));
		cztseccnt  = (double*)malloc(size * sizeof(double));
		pha  = (unsigned short*)malloc(size * sizeof(unsigned short));
		cztntick  = (unsigned short*)malloc(size * sizeof(unsigned short));
		veto  = (unsigned short*)malloc(size * sizeof(unsigned short));
		detid  = (unsigned char*)malloc(size * sizeof(unsigned char));
		pixid  =(unsigned char*) malloc(size * sizeof(unsigned char));
		detx  = (unsigned char*)malloc(size * sizeof(unsigned char));
		dety  = (unsigned char*)malloc(size * sizeof(unsigned char));
		alpha = (unsigned char*)malloc(size * sizeof(unsigned char));
		energy = (float*)malloc(sizeof(float)*size);
		pi = (int*)malloc(sizeof(int)*size);

		finalevttime  = (double*)malloc(size * sizeof(double));
		finalcztseccnt  = (double*)malloc(size * sizeof(double));
		finalpha  = (unsigned short*)malloc(size * sizeof(unsigned short));
		finalcztntick  = (unsigned short*)malloc(size * sizeof(unsigned short));
		finalveto  = (unsigned short*)malloc(size * sizeof(unsigned short));
		finaldetid  = (unsigned char*)malloc(size * sizeof(unsigned char));
		finalpixid  =(unsigned char*) malloc(size * sizeof(unsigned char));
		finaldetx  = (unsigned char*)malloc(size * sizeof(unsigned char));
		finaldety  = (unsigned char*)malloc(size * sizeof(unsigned char));
		finalalpha = (unsigned char*)malloc(size * sizeof(unsigned char));
		finalenergy = (float*)malloc(sizeof(float)*size);
		finalpi = (int*)malloc(sizeof(int)*size);

	FILE *fp;
	
	char* tempevt = calloc(strlen(evtfile)+1, sizeof(char));
	strcpy(tempevt, evtfile);

	if ( fits_open_file(&caldbfptr, lldThresholdFile, READONLY, &status) )
	{
	    printf("Error(%s:%d)  in opening a file : %s",__FILE__,__LINE__, lldThresholdFile);
	    printerror( status );
	}

	/*fits_get_colnum(caldbfptr, CASEINSEN, "DETID",&llddetidcolnum, &status);
	fits_get_colnum(caldbfptr, CASEINSEN, "PIXID", &lldpixidcolnum, &status);
	fits_get_colnum(caldbfptr, CASEINSEN, "LLD",&lldcolnum, &status);*/

	status=0;
	if(fits_open_file(&bunchfptr,bunchfile,READONLY,&status))
	{ 
		
		printf("Error(%s:%d)  in opening a file : %s",__FILE__,__LINE__, bunchfile);
		printerror( status );
	}

	/*fits_get_colnum(bunchfptr, CASEINSEN, "Time",&bunchtimecolnum, &status);
	fits_get_colnum(bunchfptr, CASEINSEN, "NumEvent", &numeventcolnum, &status);
	fits_get_colnum(bunchfptr, CASEINSEN, "Detid1",&bunchdetidcolnum, &status);*/

	if ( fits_open_file(&evtfptr, evtfile, READONLY, &status) )
	{
	    	printf("Error(%s:%d)  in opening a file : %s",__FILE__,__LINE__, evtfile);
	   	printerror( status );
	} 

	/*fits_get_colnum(evtfptr, CASEINSEN, "Time", &timecolnum, &status);
	fits_get_colnum(evtfptr, CASEINSEN, "CZTSECCNT", &cztseccntcolnum, &status);
	fits_get_colnum(evtfptr, CASEINSEN, "CZTNTICK",&cztntickcolnum, &status);
	fits_get_colnum(evtfptr, CASEINSEN, "PHA",&phacolnum, &status);
	fits_get_colnum(evtfptr, CASEINSEN, "DetID", &detidcolnum, &status);
	fits_get_colnum(evtfptr, CASEINSEN, "pixID",&pixidcolnum, &status);
	fits_get_colnum(evtfptr, CASEINSEN, "DETX",&detxcolnum, &status);
	fits_get_colnum(evtfptr, CASEINSEN, "DETY", &detycolnum, &status);
	fits_get_colnum(evtfptr, CASEINSEN, "veto",&vetocolnum, &status);
	fits_get_colnum(evtfptr, CASEINSEN, "alpha",&alphacolnum, &status);
	fits_get_colnum(evtfptr, CASEINSEN, "pi",&picolnum, &status);
	fits_get_colnum(evtfptr, CASEINSEN, "energy",&energycolnum, &status);
	*/
	//create event and badpix file
	char *file = strtok(evtfile, "."); 
	bzero(outevtfile,sizeof(outevtfile));
	sprintf(outevtfile, "%s_hbc.evt",file);
	
	remove(outevtfile);
        printf("filename : %s\n",outevtfile);
	createEventFile(outevtfile,tempevt);
	
	sprintf(outlogfile, "%s_hbc.log",file);
	remove(outlogfile);
	logfile=fopen(outlogfile,"a"); 

	//fprintf(logfile,"Inputs for Heavy bunch clean\n1.Event file : %s\n2.Bunch file : %s\n3.LLD Threshold file : %s\n\n",tempevt,bunchfile,lldThresholdFile);
	//fprintf(logfile,"Important thresholds ....\n1.bun_size_thresh = %d\n2.heavy_bunch_time_threshold_LLD = %f\n3.heavy_bunch_time_threshold_all = %f\n4.module_lld_threshold : %f\n\n",bun_size_thresh,heavy_bunch_time_threshold_LLD,heavy_bunch_time_threshold_all,module_lld_threshold);

	double **pixel_exposure;
	pixel_exposure=(double**)malloc(sizeof(double*)*4);
	for(i=0;i<4;i++) pixel_exposure[i]=(double*)malloc(sizeof(double)*4096);
    //for(qid=0;qid<4;qid++){for(j=0;j<4096;j++) pixel_exposure[qid][j]=0.0;}

	int LLD[5],det,pix;
	for(qid=quadstart; qid<=quadend; qid++)
	{
		printf("Q%d Start\n",qid);
		
		double pix_exposure[64][64], pix_btistop_prev[64][64];
		//fprintf(logfile,"Quad %d is processing...\n",qid);
		status=0;
		fits_movabs_hdu(caldbfptr, qid+2, NULL, &status);
	    	fits_get_num_rows(caldbfptr, &nrows, &status);
		
		caldb_detid  = (unsigned char*)realloc(caldb_detid,nrows * sizeof(unsigned char));
		caldb_pixid  = (unsigned char*)realloc(caldb_pixid,nrows * sizeof(unsigned char));
		caldb_detx  = (unsigned char*)realloc(caldb_detx,nrows * sizeof(unsigned char));
		caldb_dety  = (unsigned char*)realloc(caldb_dety,nrows * sizeof(unsigned char));
		caldb_lld  = (int*)realloc(caldb_lld,nrows * sizeof(int));

		frow	  = 1;	    	
		felem     = 1;
	    bytenull  = 0;


		float LLD_thresh[64][64];
	
	  	fits_read_col(caldbfptr, TBYTE, 1, frow, felem, nrows, &bytenull, caldb_detid,&anynull, &status);    
		fits_read_col(caldbfptr, TBYTE, 2, frow, felem, nrows, &bytenull, caldb_pixid,&anynull, &status);
		
		
		for(det=0;det<16;det++)  // For each detector in the quadrant
        {
			
			for (pix=0;pix<256;pix++)   // For each pixel in the module
            {
				frow=det*256+pix; 
				fits_read_col(caldbfptr,TINT,3,frow+1,felem,5,&intnull,LLD,&anynull,&status);
				caldb_lld[frow]=LLD[1];

				caldb_detx[frow]=((caldb_detid[frow]%4)*16)+(caldb_pixid[frow]%16);
				caldb_dety[frow]=((caldb_detid[frow]/4)*16)+(caldb_pixid[frow]/16);
			
				if(qid==0 || qid==3)
					caldb_dety[frow]=63-caldb_dety[frow];
				else
					caldb_detx[frow]=63-caldb_detx[frow];
				LLD_thresh[caldb_detx[frow]][caldb_dety[frow]]=(caldb_lld[frow]*0.5)+5.0+module_lld_threshold;
				//printf("%f\n",LLD_thresh[caldb_detx[frow]][caldb_dety[frow]]);
			}

		}

		status=0;
		fits_movabs_hdu(bunchfptr, qid+2, NULL, &status);
	    fits_get_num_rows(bunchfptr, &bunchnrows, &status);
  		
		bunchtime  = (double*)realloc(bunchtime,bunchnrows * sizeof(double));
		bunsize  = (int*)realloc(bunsize,bunchnrows * sizeof(int));
		buntime_real = (double*)realloc(bunchtime,bunchnrows * sizeof(double));
		bunsize_real = (int*)realloc(bunsize,bunchnrows * sizeof(int));
		
		bunch_detid1  = (unsigned char*)realloc(bunch_detid1,bunchnrows * sizeof(unsigned char));
		bunch_detid2  = (unsigned char*)realloc(bunch_detid2,bunchnrows * sizeof(unsigned char));
		bunch_detid3  = (unsigned char*)realloc(bunch_detid3,bunchnrows * sizeof(unsigned char));
		bunch_detid4  = (unsigned char*)realloc(bunch_detid4,bunchnrows * sizeof(unsigned char));
		bunch_detid   = (unsigned char*)realloc(bunch_detid,bunchnrows * sizeof(unsigned char));
		buntime_dfs   = (unsigned char*)realloc(buntime_dfs,sizeof(unsigned char)*bunchnrows);
		buntime_dsl   = (unsigned char*)realloc(buntime_dsl,sizeof(unsigned char)*bunchnrows);
		

		frow      = 1;
		felem     = 1;
		doublenull = 0.;

	  	fits_read_col(bunchfptr, TDOUBLE, 1 , frow, felem, bunchnrows, &doublenull, bunchtime,&anynull, &status);    
		fits_read_col(bunchfptr, TINT, 4, frow, felem, bunchnrows, &intnull, bunsize,&anynull, &status);
		fits_read_col(bunchfptr,TBYTE,5,frow,felem,bunchnrows,&intnull,bunch_detid1,&anynull,&status);
		fits_read_col(bunchfptr,TBYTE,6,frow,felem,bunchnrows,&intnull,bunch_detid2,&anynull,&status);
		fits_read_col(bunchfptr,TBYTE,7,frow,felem,bunchnrows,&intnull,bunch_detid3,&anynull,&status);
		fits_read_col(bunchfptr,TBYTE,8,frow,felem,bunchnrows,&intnull,bunch_detid4,&anynull,&status);
		fits_read_col(bunchfptr, TBYTE, 2, 1, 1, bunchnrows, NULL, buntime_dfs,NULL, &status);
		fits_read_col(bunchfptr, TBYTE, 3, 1, 1, bunchnrows, NULL, buntime_dsl,NULL, &status);
		
		

		status=0;
		fits_movabs_hdu(evtfptr, qid+2, NULL, &status);
		fits_get_num_rows(evtfptr, &evtnrows, &status);

		evttime  = (double*)realloc(evttime,evtnrows * sizeof(double));
		cztseccnt  = (double*)realloc(cztseccnt,evtnrows * sizeof(double));
		pha  = (unsigned short*)realloc(pha,evtnrows * sizeof(unsigned short));
		cztntick  = (unsigned short*)realloc(cztntick,evtnrows * sizeof(unsigned short));
		veto  = (unsigned short*)realloc(veto,evtnrows * sizeof(unsigned short));
		detid  = (unsigned char*)realloc(detid,evtnrows * sizeof(unsigned char));
		pixid  =(unsigned char*) realloc(pixid,evtnrows * sizeof(unsigned char));
		detx  = (unsigned char*)realloc(detx,evtnrows * sizeof(unsigned char));
		dety  = (unsigned char*)realloc(dety,evtnrows * sizeof(unsigned char));
		alpha = (unsigned char*)realloc(alpha,evtnrows * sizeof(unsigned char));
		energy = (float*)realloc(energy,sizeof(float)*evtnrows);
		pi = (int*)realloc(pi,sizeof(int)*evtnrows);
		

		finalevttime  = (double*)realloc(finalevttime,evtnrows * sizeof(double));
		finalcztseccnt  = (double*)realloc(finalcztseccnt,evtnrows * sizeof(double));
		finalpha  = (unsigned short*)realloc(finalpha,evtnrows * sizeof(unsigned short));
		finalcztntick  = (unsigned short*)realloc(finalcztntick,evtnrows * sizeof(unsigned short));
		finalveto  = (unsigned short*)realloc(finalveto,evtnrows * sizeof(unsigned short));
		finaldetid  = (unsigned char*)realloc(finaldetid,evtnrows * sizeof(unsigned char));
		finalpixid  =(unsigned char*) realloc(finalpixid,evtnrows * sizeof(unsigned char));
		finaldetx  = (unsigned char*)realloc(finaldetx,evtnrows * sizeof(unsigned char));
		finaldety  = (unsigned char*)realloc(finaldety,evtnrows * sizeof(unsigned char));
		finalalpha = (unsigned char*)realloc(finalalpha,evtnrows * sizeof(unsigned char));
		finalenergy = (float*)realloc(finalenergy,sizeof(float)*evtnrows);
		finalpi = (int*)realloc(finalpi,sizeof(int)*evtnrows);

		
								
		frow      = 1;
		felem     = 1;
		doublenull = 0.;
		intnull = 0;
		bytenull = 0;
		floatnull = 0.0;

		fits_read_col(evtfptr, TDOUBLE, 1, frow, felem, evtnrows, &doublenull, evttime,&anynull, &status);
		fits_read_col(evtfptr, TDOUBLE, 2, frow, felem, evtnrows, &doublenull, cztseccnt, &anynull, &status);         
		fits_read_col(evtfptr, TUSHORT, 3, frow, felem, evtnrows, &intnull, cztntick, &anynull, &status); 
		fits_read_col(evtfptr, TUSHORT, 4, frow, felem, evtnrows, &intnull, pha,&anynull, &status);
		fits_read_col(evtfptr, TBYTE, 5, frow, felem,evtnrows, &bytenull, detid, &anynull, &status);  
		fits_read_col(evtfptr, TBYTE, 6, frow, felem,evtnrows, &bytenull, pixid, &anynull, &status);        
		fits_read_col(evtfptr, TBYTE, 7, frow, felem, evtnrows, &bytenull, detx, &anynull, &status);   
		fits_read_col(evtfptr, TBYTE, 8, frow, felem,evtnrows, &bytenull, dety,&anynull, &status);
		fits_read_col(evtfptr, TUSHORT, 9, frow, felem,evtnrows, &intnull, veto,&anynull, &status);
		fits_read_col(evtfptr, TBYTE, 10, frow, felem, evtnrows, &bytenull, alpha,&anynull, &status); 
		fits_read_col(evtfptr, TINT, 11, frow, felem, evtnrows, &floatnull, pi,&anynull, &status);
		fits_read_col(evtfptr, TFLOAT, 12, frow, felem, evtnrows, &floatnull, energy,&anynull, &status);
		
		double tstart,tstop,tot_exposure;
		fits_read_key(evtfptr,TDOUBLE,"TSTART",&tstart,NULL, &status);
		fits_read_key(evtfptr,TDOUBLE,"TSTOP",&tstop,NULL, &status);
		fits_read_key(evtfptr,TDOUBLE,"EXPOSURE",&tot_exposure,NULL, &status);
		

		event_flag  = (int*)calloc(sizeof(int),evtnrows);
		bunch_flag  = (int*)calloc(sizeof(int),evtnrows);
		
		int heavy_bunch_length=0,bundet_heavy_temp=0,temp,small_bunch_length=0;
		double *buntime_heavy,*buntime_small;
		unsigned char *bundet_small,*bundet_heavy;

		bundet_heavy  = (unsigned char*)malloc(bunchnrows * sizeof(unsigned char));
		bundet_small  = (unsigned char*)malloc(bunchnrows * sizeof(unsigned char));

		buntime_heavy  = (double*)malloc(bunchnrows * sizeof(double));
		buntime_small  = (double*)malloc(bunchnrows * sizeof(double));


		

		int real_bunch_length=0;
		
		int k;
		i=0;
		while(i < bunchnrows)
		{
			if((bunchtime[i]<(tstart-1.0)) || (bunchtime[i]>tstop))
			{
				i++;
				continue;	
			}
			
			for(j=0;j<16;j++){bunch_det_count[j]=0;}

			
			
			buntime_real[real_bunch_length] = bunchtime[i];
			bunsize_real[real_bunch_length] = bunsize[i];
			if(bunsize[i]!=63)
			{
				bundet_heavy_temp = bunch_det_count[0];
				temp = 0;
				for(k=1;k<16;k++)
				{
					if(bunch_det_count[k]>bundet_heavy_temp)
					{
						bundet_heavy_temp = bunch_det_count[k];
						temp = k;
					}
			
				}
				bunch_detid[real_bunch_length]= temp;
				//printf("%u\n",bunch_detid[real_bunch_length]);
				
				
				if(bunsize_real[real_bunch_length] >= bun_size_thresh)
				{ 
				
					buntime_heavy[heavy_bunch_length] = buntime_real[real_bunch_length];
					bundet_heavy[heavy_bunch_length] = bunch_detid[real_bunch_length];
					heavy_bunch_length++;
				
				}
				else
				{
					buntime_small[small_bunch_length] = buntime_real[real_bunch_length];
					bundet_small[small_bunch_length] = bunch_detid[real_bunch_length];
					small_bunch_length++;
					
				}
				
				
				
				
				real_bunch_length++;
				i++;
				continue;
			}
			
			
			else
			{
				for(j=i+1;j<bunchnrows;j++)
				{
					
					bunch_det_count[bunch_detid1[i]]++;
					bunch_det_count[bunch_detid2[i]]++;
					bunch_det_count[bunch_detid3[i]]++;
					if(bunsize[i] >= 4)bunch_det_count[bunch_detid4[i]]++;
					
					
					
					if( ((bunchtime[j] - 20.0*(double)buntime_dfs[j]/1000000.0)-(bunchtime[i] + 20.0*(double)buntime_dsl[i]/1000000.0)) < 30.0/1000000.0)
					{
						//printf("T\n");
						i=j;
						buntime_real[real_bunch_length] = (bunchtime[j] + 20.0*(double)buntime_dsl[j]/1000000.0);
						bunsize_real[real_bunch_length] = bunsize_real[real_bunch_length]+bunsize[j];
			
					}
					else
					{
						bundet_heavy_temp = bunch_det_count[0];
						temp = 0;
						for(k=1;k<16;k++)
						{
							if(bunch_det_count[k]>bundet_heavy_temp)
							{
								bundet_heavy_temp = bunch_det_count[k];
								temp = k;
							}
			
						}
						bunch_detid[real_bunch_length]= temp;
						//printf("%u\n",bunch_detid[real_bunch_length]);
						
						
						if(bunsize_real[real_bunch_length] >= bun_size_thresh)
						{ 
						
							buntime_heavy[heavy_bunch_length] = buntime_real[real_bunch_length];
							bundet_heavy[heavy_bunch_length] = bunch_detid[real_bunch_length];
							heavy_bunch_length++;
						
						}
						else
						{
							buntime_small[small_bunch_length] = buntime_real[real_bunch_length];
							bundet_small[small_bunch_length] = bunch_detid[real_bunch_length];
							small_bunch_length++;
							
						}
						
						
		
						
						i++;
						real_bunch_length++;
						break;
					}
				}
			}
			//printf("%lf %d\n",buntime_real[real_bunch_length-1],bunsize_real[real_bunch_length-1]);
			//printf("%ld %d\n",i,bunnrows);
			//printf("%d\n",kk);
			
		}
		printf("Bunch length correction Completed\n");
		
		/*
		for(i=0;i < bunchnrows;i++)
		{
			//printf("index %d %d %d %d %lf\n",qid,i,bunchnrows,bunsize[i],bunchtime[i]);
						
			for(j=0;j<16;j++)
				bunch_det_count[j]=0;

			bunch_det_count[bunch_detid1[i]]++;
			bunch_det_count[bunch_detid2[i]]++;
			bunch_det_count[bunch_detid3[i]]++;
			if(bunsize[i] >= 4)bunch_det_count[bunch_detid4[i]]++;
			
			bundet_heavy_temp = bunch_det_count[0];
			temp = 0;
			for(j=1;j<16;j++)
			{
				if(bunch_det_count[j]>bundet_heavy_temp)
				{
					bundet_heavy_temp = bunch_det_count[j];
					temp = j;
				}

			}
			bunch_detid[i]= temp;
			
			if(bunsize[i] >= bun_size_thresh)
			{ 
				//printf("buntime %f\n",bunchtime[i]-bunchtime[0]);
				buntime_heavy[heavy_bunch_length] = bunchtime[i];
				bundet_heavy[heavy_bunch_length] = bunch_detid[i];
				heavy_bunch_length++;
				//printf("%ld\n",heavy_bunch_length);
			}
			else
			{
				buntime_small[small_bunch_length] = bunchtime[i];
				bundet_small[small_bunch_length] = bunch_detid[i];
				small_bunch_length++;
				//printf("%ld\n",small_bunch_length);
			}
		}
		
		*/
		
		//printf("buntime_heavy %f\t heavy_bunch_length %d\n",buntime_heavy[heavy_bunch_length-1]-buntime_heavy[0],heavy_bunch_length);
		int prev =0;
		k=0;
		//fprintf(logfile,"Events are recorded after heavy bunches\nRemoved events\nIndex\tTime\tDETX\tDETY\n");
		unsigned char detx_min, detx_max, dety_min, dety_max,detx_temp,dety_temp;
		int m, p;
		
		//initiallizing Pixel Exposures 
		
		for(i=0;i<64;i++)
		{
			for(j=0;j<64;j++)
			{
				pix_exposure[i][j] = 0.0;
				pix_btistop_prev[i][j] = tstart-1.0;
			}
			
		}
		
		
		
		
		
		for(i=0;i<heavy_bunch_length;i++)
		{
			//convert detid to detx,dety
			//bundet_heavy[i]
			if(((tstart-1.0)<buntime_heavy[i]) && (buntime_heavy[i]<tstop))
			{
			
				
				detx_temp=((bundet_heavy[i]%4)*16)+(119%16);
				dety_temp=((bundet_heavy[i]/4)*16)+(119/16);
				
				if(qid ==0 ||qid ==3)
					dety_temp=63-dety_temp;
				else
					detx_temp=63-detx_temp;
					
					
				if(detx_temp>12)
				{
				detx_min = detx_temp-12;
				detx_max = detx_temp+13;
				}
				else 
				{
				detx_min = 0;
				detx_max = detx_temp+13;	
				}
				if(dety_temp>12)
				{
				dety_min = dety_temp-12;
				dety_max = dety_temp+13;
				}
				else 
				{
				dety_min = 0;
				dety_max = dety_temp+13;	
				}
				if(dety_max>63)
				{
					dety_max = 63;
				
				}
				if(detx_max>63)
				{
					detx_max = 63;
				
				}
				
				
				for(m=detx_min;m <= detx_max;m++)
				{
					for(p=dety_min;p <= dety_max;p++)
					{
						if(pix_btistop_prev[m][p] < buntime_heavy[i])
						{
							//printf("%lf\t",pix_exposure[m][p]);
							pix_exposure[m][p] = pix_exposure[m][p]+heavy_bunch_time_threshold_all;
							//printf("%lf\t%lf\n",pix_exposure[m][p],heavy_bunch_time_threshold_all);
							pix_btistop_prev[m][p] = buntime_heavy[i]+heavy_bunch_time_threshold_all;
							//printf("%lf\t%lf\n",pix_exposure[m][p],pix_btistop_prev[m][p]);
						}
						else
						{
							pix_exposure[m][p] = pix_exposure[m][p]+(buntime_heavy[i]+heavy_bunch_time_threshold_all-pix_btistop_prev[m][p]);
							//printf("%lf\t%lf\t%lf\n",(buntime_heavy[i]+heavy_bunch_time_threshold_all-pix_btistop_prev[m][p]),pix_btistop_prev[m][p],buntime_heavy[i]);
							//printf("%lf\t%lf\n",pix_exposure[m][p],pix_btistop_prev[m][p]);
							pix_btistop_prev[m][p] = buntime_heavy[i]+heavy_bunch_time_threshold_all;
						}
						
					}
					
				}
				
				
				
				//printf("%d\t%u\t%u\t%u\t%u\t%u\t%u\t%u\n",qid,bundet_heavy[i],detx_temp,dety_temp,detx_min,detx_max,dety_min,dety_max);
				//(( detx_min <= detx[j] && detx_max >= detx[j]) && ( dety_min <= dety[j] && dety_max >= dety[j]))
				
				for(j=prev;j<evtnrows;j++)
				{
					if(!(( detx_min <= detx[j] && detx_max >= detx[j]) && ( dety_min <= dety[j] && dety_max >= dety[j])) && (bundet_heavy[i]==detid[j])){printf("%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t\n",bundet_heavy[i],detx[j],dety[j],detid[j],pixid[j],detx_min,detx_max,dety_min,dety_max);}	
					//if((evttime[j]>=buntime_heavy[i] && evttime[j]<=buntime_heavy[i]+heavy_bunch_time_threshold_LLD) && (bundet_heavy[i]==detid[j]) && (energy[j]<LLD_thresh[detx[j]][dety[j]]))
					if((evttime[j]>=buntime_heavy[i] && evttime[j]<=buntime_heavy[i]+heavy_bunch_time_threshold_LLD) && (( detx_min <= detx[j] && detx_max >= detx[j]) && ( dety_min <= dety[j] && dety_max >= dety[j])) && (energy[j]<LLD_thresh[detx[j]][dety[j]]))
					{
						//printf("%lf\n",LLD_thresh[detx[j]][dety[j]]-energy[j]);
						//event_flag[j]=1;//comment when not required
				
						for(k=j+1;k<evtnrows;k++)
						{
							if(!(( detx_min <= detx[k] && detx_max >= detx[k]) && ( dety_min <= dety[k] && dety_max >= dety[k])) && (bundet_heavy[i]==detid[k])){printf("%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t\n",bundet_heavy[i],detx[k],dety[k],detid[k],pixid[k],detx_min,detx_max,dety_min,dety_max);}
							
							//if((evttime[k]>evttime[j] && evttime[k]<=(evttime[j]+0.002)) && (bundet_heavy[i]==detid[k]) && (energy[k]<LLD_thresh[detx[k]][dety[k]]))
							if((evttime[k]>evttime[j] && evttime[k]<=(evttime[j]+0.002)) && (( detx_min <= detx[k] && detx_max >= detx[k]) && ( dety_min <= dety[k] && dety_max >= dety[k])) && (energy[k]<LLD_thresh[detx[k]][dety[k]]))
							{
								//printf("true\n");
								if(evttime[j]-buntime_heavy[i+1]<0.0){prev =j;}
								//prev =j;
								event_flag[j]=1;
								event_flag[k]=1;
								
								break;
	
							}
	
							if(evttime[k]-evttime[j] > 0.01){break;}
	
						}
	
					}
					
					
					//if((evttime[j]>=buntime_heavy[i] && evttime[j]<=buntime_heavy[i]+heavy_bunch_time_threshold_all) && (bundet_heavy[i]==detid[j]))
					if((evttime[j]>=buntime_heavy[i] && evttime[j]<=buntime_heavy[i]+heavy_bunch_time_threshold_all) && (( detx_min <= detx[j] && detx_max >= detx[j]) && ( dety_min <= dety[j] && dety_max >= dety[j])))
					{
						if(evttime[j]-buntime_heavy[i+1]<0.0){prev =j;}
						event_flag[j]=1;
					}
					if(evttime[j]-buntime_heavy[i] > 1.0){break;}
				}
			}
		}
		
	    
		prev=0;
		for(i=0;i<small_bunch_length;i++)
		{
			
			if(((tstart-1.0)<bundet_small[i]) && (bundet_small[i]<tstop))
			{
				
				detx_temp=((bundet_small[i]%4)*16)+(119%16);
				dety_temp=((bundet_small[i]/4)*16)+(119/16);
				
				if(qid ==0 ||qid ==3)
					dety_temp=63-dety_temp;
				else
					detx_temp=63-detx_temp;
					
					
				if(detx_temp>12)
				{
				detx_min = detx_temp-12;
				detx_max = detx_temp+13;
				}
				else 
				{
				detx_min = 0;
				detx_max = detx_temp+13;	
				}
				if(dety_temp>12)
				{
				dety_min = dety_temp-12;
				dety_max = dety_temp+13;
				}
				else 
				{
				dety_min = 0;
				dety_max = dety_temp+13;	
				}
				if(dety_max>63)
				{
					dety_max = 63;
				
				}
				if(detx_max>63)
				{
					detx_max = 63;
				
				}
				
				
				
				
				for(m=detx_min;m <= detx_max;m++)
				{
					for(p=dety_min;p <= dety_max;p++)
					{
						if(pix_btistop_prev[m][p] < buntime_small[i])
						{
							pix_exposure[m][p] = pix_exposure[m][p]+bunch_time_threshold_small;
							pix_btistop_prev[m][p] = buntime_small[i]+bunch_time_threshold_small;
				
						}
						else
						{
							pix_exposure[m][p] = pix_exposure[m][p]+(buntime_small[i]+bunch_time_threshold_small-pix_btistop_prev[m][p]);
							pix_btistop_prev[m][p] = buntime_small[i]+heavy_bunch_time_threshold_all;
						}
						
					}
					
				}
			
				
				
	
				for(j=prev;j<evtnrows;j++)
				{
					if((evttime[j]>=buntime_small[i] && evttime[j]<=buntime_small[i]+bunch_time_threshold_small) && (( detx_min <= detx[j] && detx_max >= detx[j]) && ( dety_min <= dety[j] && dety_max >= dety[j])))// && (energy[j]<LLD_thresh[detx[j]][dety[j]]))
					{
						if(evttime[j]-buntime_small[i+1]<0.0){prev =j;}
						
						event_flag[j]=1;
					}
					if(evttime[j]-buntime_small[i] > 0.1){break;}
				}
			}
		
		}
		printf("Main Process Over\n");
		/*
		for(i=0;i<64;i++)
		{
			for(j=0;j<64;j++)
			{
				//printf("%lf\t",pix_exposure[i][j]);
				pix_exposure[i][j] = (tot_exposure-pix_exposure[i][j])/tot_exposure;
				
				//printf("%lf\n",pix_exposure[i][j]);
				//printf("%lf\n",pix_btistop_prev[i][j]);
			}
			
		}*/
		
		
		
		
		//double pix_exposure_det_pix[16][256];
		unsigned char detx_t,dety_t;
		
		//for(i=0;i<16;i++)
		//{
		//	for(j=0;j<256;j++)
		//	{
		//		pix_exposure_det_pix[i][j] = 0;
		//	}
		//	
		//}
		
		
		frow      = 1;
		felem     = 1;
		doublenull = 0.;
		
		fits_movabs_hdu(evtfptr,14, NULL, &status);
		fits_read_col(evtfptr, TDOUBLE, qid+1, frow, felem, 4096, &doublenull, pixel_exposure[qid],&anynull, &status);
		
		
		
		
		/*
		for(i=0;i<4096;i++)
		{
			printf("%lf\n",pixel_exposure[qid][i]);
		}*/
		
		
		for(i=0;i<16;i++)
		{
			for(j=0;j<256;j++)
			{
				detx_t = ((i%4)*16)+(j%16);
				dety_t = ((i/4)*16)+(j/16);
				
				if(qid ==0 ||qid ==3)
					dety_t=63-dety_t;
				else
					detx_t=63-detx_t;
				
				//pix_exposure_det_pix[i][j] = pix_exposure[detx_t][dety_t];
				
				if(pixel_exposure[qid][i*256+j]!=0.0)
				{
					pixel_exposure[qid][i*256+j] = (pixel_exposure[qid][i*256+j]*tot_exposure-pix_exposure[detx_t][dety_t])/tot_exposure ;
					
				}
				
				//printf("%lf\t%lf\n",pix_exposure_det_pix[i][j],pixel_exposure[qid][i*256+j]);
				
			}
			
		}
		
		modifyExposure(outevtfile,pixel_exposure,qid);
		
		int l=0;
		//sum=0;

		for(i=0;i<evtnrows;i++)
		{
			if(event_flag[i]==0)
			{
				finalevttime[l]=evttime[i];
				finalcztseccnt[l]  = cztseccnt[i];
				finalpha[l]  = pha[i];
				finalcztntick[l]  = cztntick[i];
				finalveto[l]  = veto[i];
				finaldetid[l]  = detid[i];
				finalpixid[l]  = pixid[i];
				finaldetx[l]  = detx[i];
				finaldety[l]  = dety[i];
				finalalpha[l] = alpha[i];
				finalenergy[l] = energy[i];
				finalpi[l] = pi[i];
				l++;
			}
			else if(event_flag[i]==1)
				fprintf(logfile,"%d\t%d\t%lf\t%f\t%u\t%u\t%u\t%u\t%f\n",i,qid,evttime[i],energy[i],detx[i],dety[i],detid[i],pixid[i],LLD_thresh[detx[i]][dety[i]]-module_lld_threshold);
		
		}
		writeEvent(finalevttime,finalcztseccnt,finalcztntick,finalpha,finaldetid,finalpixid,finaldetx,finaldety,finalveto,finalalpha,finalpi,finalenergy,outevtfile,l,qid+2);
		printf("Q%d End\n",qid);
	}
	modifyEventHeaderParams(outevtfile);
	
	return;
}

//creating event file 
void createEventFile(char *outputfile,char *eventfile)
{
	
	fitsfile *fptrOut,*fptrevt;      
	int status, hdutype,anynull,tfields=12,i,hdunum=2;
	long frow, felem;
	int mjdrefi=55197,mjdreff=0,equinox=2000;
	float ra_pnt,dec_pnt;
	double timedel,telapse,doublenull;
	char object[20],obs_id[20],obs_mode[20],date_obs[20],time_obs[20],date_end[20],time_end[20],date[20],creator[20],filename[70],checksum[20],datasum[20],chksumcomm[50],datasumcomm[50];
      
	char extname[20]; 
	          
	
	char *ttype[] = { "TIME", "CZTSECCNT","CZTNTICK","PHA","DetID","pixID","DETX","DETY","veto","alpha","PI","ENERGY"};
	char *tform[] = { "D","D","I","I","B","B","B","B","I","B","I","E"};
	char *tunit[] = {"s","s","micro-sec","counts","","","","","counts","counts","",""};
       
	status=0;
        if (fits_create_file(&fptrOut, outputfile, &status))
	       	 printerror( status );       

	if ( fits_open_file(&fptrOut, outputfile, READWRITE, &status) ) 
	        printerror( status );

	status=0;
	
	if ( fits_open_file(&fptrevt,eventfile, READONLY, &status) ) 
	         printerror( status );

	// Copy Primary
        if(fits_movabs_hdu(fptrevt, 1, 0, &status)) 
		printerror( status );         

	if(fits_copy_hdu(fptrevt, fptrOut, 0, &status))
		printerror( status );

	if ( fits_close_file(fptrOut, &status) )       
	        printerror( status );
	
    	for(i=0;i<=3;i++)
    	{      
		status=0;
		sprintf(extname, "Q%d",i);
		strcpy(chksumcomm,"HDU checksum updated ");
		strcpy(datasumcomm,"Data unit checksum updated ");
		
		if(fits_movabs_hdu(fptrevt,i+hdunum, 0, &status)) 
			printerror( status );  

		
		fits_read_key(fptrevt,TSTRING,"OBJECT",&object,NULL, &status);
		fits_read_key(fptrevt,TFLOAT,"RA_PNT",&ra_pnt,NULL, &status);
		fits_read_key(fptrevt,TFLOAT,"DEC_PNT",&dec_pnt,NULL, &status);
		fits_read_key(fptrevt,TSTRING,"OBS_ID",&obs_id,NULL, &status);
		fits_read_key(fptrevt,TSTRING,"OBS_MODE",&obs_mode,NULL, &status);
		fits_read_key(fptrevt,TSTRING,"DATE-OBS",&date_obs,NULL, &status);
		fits_read_key(fptrevt,TSTRING,"TIME-OBS",&time_obs,NULL, &status);
		fits_read_key(fptrevt,TSTRING,"DATE-END",&date_end,NULL, &status);		
		fits_read_key(fptrevt,TSTRING,"TIME-END",&time_end,NULL, &status);
		fits_read_key(fptrevt,TDOUBLE,"TIMEDEL",&timedel,NULL, &status);
		fits_read_key(fptrevt,TDOUBLE,"TELAPSE",&telapse,NULL, &status);
		fits_read_key(fptrevt,TSTRING,"DATE",&date,NULL , &status);	
		fits_read_key(fptrevt,TSTRING,"FILENAME",&filename,NULL, &status);
		fits_read_key(fptrevt,TSTRING,"CHECKSUM",&checksum,NULL, &status);
		fits_read_key(fptrevt,TSTRING,"DATASUM",&datasum,NULL, &status);
		//printf("DATE : %s\n",)

		status=0;

		if ( fits_open_file(&fptrOut, outputfile, READWRITE, &status) ) 
	        	printerror( status );
		if ( fits_create_tbl( fptrOut, BINARY_TBL, 0, tfields, ttype, tform,tunit, extname, &status) )
			printerror( status );
		if ( fits_movabs_hdu(fptrOut, i+hdunum, &hdutype, &status) ) 
	         	printerror( status );

		fits_write_key(fptrOut,TSTRING,"OBJECT",object,"Target name", &status);
		fits_write_key(fptrOut,TFLOAT,"RA_PNT",&ra_pnt,"Nominal pointing RA", &status);
		fits_write_key(fptrOut,TFLOAT,"DEC_PNT",&dec_pnt,"Nominal pointing DEC", &status);
		fits_write_key(fptrOut,TSTRING,"OBS_ID",obs_id,"Observation ID", &status);
		fits_write_key(fptrOut,TSTRING,"OBS_MODE",obs_mode,NULL, &status);
		fits_write_key(fptrOut,TSTRING,"DATE-OBS",date_obs,"Start date of observation", &status);
		fits_write_key(fptrOut,TSTRING,"TIME-OBS",time_obs,"Start time of observation", &status);
		fits_write_key(fptrOut,TSTRING,"DATE-END",date_end,"End date of observation", &status);		
		fits_write_key(fptrOut,TSTRING,"TIME-END",time_end,"End time of observation", &status);
		fits_write_key(fptrOut,TDOUBLE,"TIMEDEL",&timedel,"Time resolution", &status);
		fits_write_key(fptrOut,TDOUBLE,"TELAPSE",&telapse,"Elapsed time", &status);	
		fits_write_key(fptrOut,TSTRING,"DATE",date,"File creation date(YY-MM-DDThh:mm:ss UT)" , &status);	
		fits_write_key(fptrOut,TSTRING,"CREATOR","dataselection","Module that created this file", &status);
		fits_write_key(fptrOut,TSTRING,"FILENAME",filename,NULL, &status);
		fits_write_key(fptrOut,TSTRING,"CHECKSUM",checksum,strcat(chksumcomm,date), &status);
		fits_write_key(fptrOut,TSTRING,"DATASUM",datasum,strcat(datasumcomm,date), &status);

		fits_write_key(fptrOut, TSTRING,"EXTNAME", extname,"Name of this binary table extension", &status);
		fits_write_key(fptrOut, TINT,"QUADID",&i,"Quadrant Number", &status);
		fits_write_key(fptrOut, TSTRING,"MISSION", "ASTROSAT","Name of the mission/satellite", &status);
		fits_write_key(fptrOut, TSTRING,"TELESCOP","ASTROSAT","Name of the mission/satellite", &status);
		fits_write_key(fptrOut, TSTRING,"INSTRUME","CZTI","Name of the instrument/detector", &status);
		fits_write_key(fptrOut, TSTRING,"ORIGIN","CZTI POC","Source of FITS FILE", &status);
		fits_write_key(fptrOut, TINT,"MJDREFI", &mjdrefi,"MJDREF Integer part", &status);
		fits_write_key(fptrOut, TINT,"MJDREFF", &mjdreff,"MJDREF Fractional part", &status);
		fits_write_key(fptrOut, TSTRING,"TIMESYS","UTC","Time in UTC", &status);
		fits_write_key(fptrOut, TINT,"EQUINOX", &equinox,"J2000", &status);
		fits_write_key(fptrOut, TSTRING,"RADECSYS", "ICRS","Reference Frame", &status);
		fits_write_key(fptrOut, TSTRING,"TIMEUNIT","s","Time is in seconds", &status);
		
		if ( fits_close_file(fptrOut, &status) )       
	        	printerror( status );
	}

	status=0;
	if ( fits_open_file(&fptrOut, outputfile, READWRITE, &status) ) 
	        	printerror( status );
	// Copy VETOSPECTRUM
        if(fits_movnam_hdu(fptrevt, BINARY_TBL, "VETOSPECTRUM", 0, &status)) 
		printerror( status );         

	if(fits_copy_hdu(fptrevt, fptrOut, 0, &status))
		printerror( status );
	
	// Copy SSM data
	if(fits_movnam_hdu(fptrevt, BINARY_TBL, "SSM Data", 0, &status)) 
		printerror( status );         

	if(fits_copy_hdu(fptrevt, fptrOut, 0, &status))
		printerror( status );

	// Copy Temp
	if(fits_movnam_hdu(fptrevt, BINARY_TBL, "TEMP", 0, &status)) 
		printerror( status );         

	if(fits_copy_hdu(fptrevt, fptrOut, 0, &status))
		printerror( status );

	// Copy GTI
	if(fits_movnam_hdu(fptrevt, BINARY_TBL, "GTI", 0, &status)) 
		printerror( status );         

	if(fits_copy_hdu(fptrevt, fptrOut, 0, &status))
		printerror( status );

	// Copy Q0_GTI
	if(fits_movnam_hdu(fptrevt, BINARY_TBL, "Q0_GTI", 0, &status)) 
		printerror( status );         

	if(fits_copy_hdu(fptrevt, fptrOut, 0, &status))
		printerror( status );

	// Copy Q1_GTI
	if(fits_movnam_hdu(fptrevt, BINARY_TBL, "Q1_GTI", 0, &status)) 
		printerror( status );         

	if(fits_copy_hdu(fptrevt, fptrOut, 0, &status))
		printerror( status );

	// Copy Q2_GTI
	if(fits_movnam_hdu(fptrevt, BINARY_TBL, "Q2_GTI", 0, &status)) 
		printerror( status );         

	if(fits_copy_hdu(fptrevt, fptrOut, 0, &status))
		printerror( status );

	// Copy Q3_GTI
	if(fits_movnam_hdu(fptrevt, BINARY_TBL, "Q3_GTI", 0, &status)) 
		printerror( status );         

	if(fits_copy_hdu(fptrevt, fptrOut, 0, &status))
		printerror( status );
	
	/*
	// Copy Exposure
	if(fits_movnam_hdu(fptrevt, BINARY_TBL, "EXPOSURE", 0, &status)) 
		printerror( status );         

	if(fits_copy_hdu(fptrevt, fptrOut, 0, &status))
		printerror( status );
	*/	
		
	int tfields_exp=4;
	
	char *ttype_exp[] = {"EXPOSURE_Q0","EXPOSURE_Q1","EXPOSURE_Q2","EXPOSURE_Q3"};
	char *tform_exp[] = { "D","D","D","D"};
	char *tunit_exp[] = {"","","",""};
	
	 
	sprintf(extname, "EXPOSURE");
	if ( fits_create_tbl( fptrOut, BINARY_TBL, 0, tfields_exp, ttype_exp, tform_exp,tunit_exp, extname, &status) )
	printerror( status );
		
	if ( fits_movabs_hdu(fptrOut, 14, &hdutype, &status) ) 
	         	printerror( status );

	if ( fits_close_file(fptrOut, &status) )       
	        printerror( status );

	if ( fits_close_file(fptrevt, &status) )       
	        printerror( status );
	return;
}

void writeEvent(double *evttime,double *cztseccnt,unsigned short *cztntick,unsigned short *pha,unsigned char *detid,unsigned char *pixid,unsigned char*detx,unsigned char *dety,unsigned short *veto,unsigned char *alpha,int *pi,float *energy,char *outputfile,int writesize,int hdunum)
{
	fitsfile *fptrOut;       
	int status, hdutype,intnull;
	long frow, felem,nrows;
	double doublenull;
	unsigned char bytenull;
	status=0;
	int tstarti,tstopi;
	double tstart,tstop,tstartf,tstopf,exposure;

	if ( fits_open_file(&fptrOut, outputfile, READWRITE, &status) ) 
	{
		printf("Error(%s:%d): Unable to open file :%s \n",__FILE__,__LINE__, outputfile);
	        printerror( status );
		exit(-1);
	}
	if ( fits_movabs_hdu(fptrOut, hdunum, &hdutype, &status) ) 
	      	printerror( status );

	tstart=evttime[0];
	tstop=evttime[writesize-1];

	tstarti=(int)tstart;
	tstopi=(int)tstop;

	tstartf=tstart-tstarti;
	tstopf=tstop-tstopi;
	exposure=tstop-tstart;

	frow      = 1;
	felem     = 1;
	doublenull = 0.;
	intnull = 0;
	bytenull = 0;
	
	fits_write_key(fptrOut, TDOUBLE,"TSTART", &tstart,"Start time of observation", &status);
	fits_write_key(fptrOut, TDOUBLE,"TSTOP", &tstop,"Stop time of observation", &status);
	fits_write_key(fptrOut, TINT,"TSTARTI", &tstarti,"Start time of observation Integer part", &status);
	fits_write_key(fptrOut, TINT,"TSTOPI", &tstopi,"Stop time of observation Integer part", &status);
	fits_write_key(fptrOut, TDOUBLE,"TSTARTF", &tstartf,"Start time of observation Fractional part", &status);
	fits_write_key(fptrOut, TDOUBLE,"TSTOPF", &tstopf,"Stop time of observation Fractional part", &status);
	fits_write_key(fptrOut, TDOUBLE,"EXPOSURE", &exposure,"Exposure time", &status);

        fits_write_col(fptrOut, TDOUBLE, 1, frow, felem, writesize, evttime,&status);
	fits_write_col(fptrOut, TDOUBLE, 2, frow, felem, writesize, cztseccnt,&status);
        fits_write_col(fptrOut, TUSHORT, 3,frow, felem, writesize, cztntick,&status);  
        fits_write_col(fptrOut, TUSHORT, 4,frow, felem, writesize, pha,&status);
	fits_write_col(fptrOut, TBYTE, 5, frow, felem, writesize,detid,&status);
        fits_write_col(fptrOut, TBYTE, 6,frow, felem, writesize, pixid,&status);
	fits_write_col(fptrOut, TBYTE, 7,frow, felem, writesize, detx,&status);
        fits_write_col(fptrOut, TBYTE, 8,frow, felem, writesize, dety,&status);
	fits_write_col(fptrOut, TUSHORT, 9, frow, felem, writesize, veto,&status);   
        fits_write_col(fptrOut, TBYTE, 10,frow, felem, writesize,alpha,&status);
	fits_write_col(fptrOut, TINT, 11,frow, felem, writesize, pi, &status);
	fits_write_col(fptrOut, TFLOAT, 12,frow, felem, writesize,energy, &status);	

	if ( fits_close_file(fptrOut, &status) )       
	        printerror( status );
	return;
}

void modifyEventHeaderParams(char *outputfile)
{
	
	fitsfile *fptrOut;  
	int status,i,tstarti,tstopi;
	double tstart,tstop,start[4],stop[4],largest,smallest,exposure,tstartf,tstopf;
	status=0;
	char *comment; 

	if ( fits_open_file(&fptrOut, outputfile, READWRITE, &status) ) 
	         printerror( status );

    	for(i=0;i<=3;i++)
    	{       
		fits_movabs_hdu(fptrOut, i+2, NULL, &status);
		fits_read_key(fptrOut,TDOUBLE,"TSTART",&tstart,NULL, &status);
		fits_read_key(fptrOut,TDOUBLE,"TSTOP",&tstop,NULL, &status);
		
		start[i]=tstart;
		stop[i]=tstop;
	}

	smallest = start[0];
	for (i = 1; i < 4; i++)
		if (smallest > start[i])
			smallest = start[i];

	largest = stop[0];
	for (i = 1; i < 4; i++)
		if (largest < stop[i])
			largest = stop[i];

	if ( fits_movabs_hdu(fptrOut, 1, NULL, &status) ) 
	      	printerror( status );
	tstarti=(int)smallest;
	tstopi=(int)largest;

	tstartf=smallest-tstarti;
	tstopf=largest-tstopi;
	exposure=largest-smallest;
	printf("Time : %f\t%f\t%d\t%d\t%10f\t%10f\n",smallest,largest,tstarti,tstopi,tstartf,tstopf);
	
	fits_update_key(fptrOut, TDOUBLE,"TSTART",&smallest,"Start time of observation",&status);
	fits_update_key(fptrOut, TDOUBLE,"TSTOP",&largest,"Stop time of observation",&status);
	fits_update_key(fptrOut, TINT,"TSTARTI", &tstarti,"Start time of observation Integer part", &status);
	fits_update_key(fptrOut, TINT,"TSTOPI", &tstopi,"Stop time of observation Integer part", &status);
	fits_update_key(fptrOut, TDOUBLE,"TSTARTF", &tstartf,"Start time of observation Fractional part", &status);
	fits_update_key(fptrOut, TDOUBLE,"TSTOPF", &tstopf,"Stop time of observation Fractional part", &status);
	fits_update_key(fptrOut, TDOUBLE,"EXPOSURE", &exposure,"Exposure time", &status);
	
	if ( fits_close_file(fptrOut, &status) )       
	        printerror( status );

	return;
}

void modifyExposure(char *outputfile, double **pix_exposure,int qid)
{
	fitsfile *fptrOut; 
	int status, hdutype,intnull;
	long frow, felem,nrows;
	double doublenull;
	unsigned char bytenull;
	status=0;
	int i;
	//double tstartf,tstopf;//,exposure=0.0;
	//int tstarti, tstopi;
	frow      = 1;
	felem     = 1;
	//tstarti = (int)tstart;
	//tstopi = (int)tstop;
	//tstartf = tstart-(double)tstarti;
	//tstopf = tstop-(double)tstopi;
	//printf("%d\t%d\n",tstarti,tstopi);
	
	if ( fits_open_file(&fptrOut, outputfile, READWRITE, &status) ) 
	        printerror( status );
	if ( fits_movabs_hdu(fptrOut, 14, &hdutype, &status) ) 
	      	printerror( status );
	      	
	fits_write_col(fptrOut, TDOUBLE, qid+1, frow, felem,4096,pix_exposure[qid],&status);
	
	
	if ( fits_close_file(fptrOut, &status) )       
	        printerror( status );
	return;
		
}



void printerror( int status)
{
	if (status)
	{
		fits_report_error(stderr, status); 	
		exit( status );    	
    	}
  	return;
}





