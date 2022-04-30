#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <complex.h>
#include <time.h>
#include <math.h>
#include <xtime_l.h>
#include "platform.h"
#include "xil_printf.h"
#include "xaxidma.h"
#include "xparameters.h"
#include "stft.h"
#include "data.h"
#include "sdCard.h"

#define N 1024

XAxiDma axiDma;

FIL* fptr; //file pointer

/*#define MEM_BASE_ADDR 0x01000000
#define RX_BUFFER_BASE (MEM_BASE_ADDR + 0x00300000)
float complex *RX_PNTR= (float complex*)RX_BUFFER_BASE;*/


int initDma()
{
	printf("Initializing DMA ... \n");
	XAxiDma_Config *DmaCfgPtr;
	DmaCfgPtr=XAxiDma_LookupConfig(XPAR_AXI_DMA_0_DEVICE_ID);

	if(!DmaCfgPtr)
	{
		xil_printf("no config found for %d\r\n",XPAR_AXI_DMA_0_DEVICE_ID);
		return XST_FAILURE;
	}
	int status=XAxiDma_CfgInitialize(&axiDma,DmaCfgPtr);

	if(status!=XST_SUCCESS)
	{
			printf("Error INITIALIAZING DMA \n");
			return XST_FAILURE;
		}
	//Disable Interrupts
	//XAxiDma_IntrDisable(&axiDma,XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DEVICE_TO_DMA);
	//XAxiDma_IntrDisable(&axiDma,XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DMA_TO_DEVICE);

	return XST_SUCCESS;

}

u32 checkIdle(u32 baseAddress,u32 offset)
{
	u32 status;
	status=(XAxiDma_ReadReg(baseAddress, offset)) & XAXIDMA_IDLE_MASK;

	return status;

}

int main()
{
	print("entering main function\n\r");
	int Status;

	int window=128;
	int overlap=32;

	initDma();




	//initialize SD CARD
	Status = SD_Init();
	if(Status != XST_SUCCESS)
		xil_printf("SD card init failed");

	fptr = openFile("stftData.csv",'w');

	if(fptr == 0)
		printf("File opening failed\n\r");

	//int RX_PNTR[N];
	float complex RX_PNTR[N];
		//float complex sfft_spec[11][1024]={{0,0}};
		//bool err_flag=False;

	//buffer to store the stft data

	//char dataBuffer[8*N]={0};
	//char *dataPntr=dataBuffer;

	char text_temp[8];
	//bool err_flag=False;





	 //DATA TRANSFER
	 int status_tf;
	 int *pFFT_input=FFT_input;
	 int len_start=lenStart(window, overlap,N);
	 xil_printf("\n length of fft inp is %d \n",sizeof(FFT_input)/sizeof(FFT_input[0]));//size of the window_index array
	 xil_printf("\n length of start is %d \n",len_start);
	 //buffer to hold the index of the the start of the window for each computation
	 int *pWin_start_index=win_start_index(window,overlap,sizeof(FFT_input)/sizeof(FFT_input[0]));
	 float complex *pWindowed_signal; //pointer to the windowed signal


	 //
	 for(int i=0;i<len_start;i++)
	 {
		 xil_printf("\ncomputing FFT for window index %d \n",i);
		 pWindowed_signal=create_windowed_signal(*(pWin_start_index+i), window, pFFT_input,N);

		 //compute the FFT of the windowed signal

		 //Dma transfer
		 Xil_DCacheFlushRange(pWindowed_signal,(sizeof(float complex)*N));
		 Xil_DCacheFlushRange((UINTPTR)RX_PNTR,(sizeof(float complex)*N));

		 xil_printf("\nDMA status before transfer to device: %d",checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x4));

		 status_tf=XAxiDma_SimpleTransfer(&axiDma,pWindowed_signal,(sizeof(float complex)*N),XAXIDMA_DMA_TO_DEVICE);
		 if(status_tf!=XST_SUCCESS)
		 {
		 	printf("WRITING DATA FROM DDR TO FIFO VIA DMA FAILED  \n");
		 }
		 status_tf= XAxiDma_SimpleTransfer(&axiDma,(UINTPTR)RX_PNTR,(sizeof(float complex)*N),XAXIDMA_DEVICE_TO_DMA);
		 if(status_tf!=XST_SUCCESS)
		 {
		 	printf("\n WRITING DATA FROM IP TO DMA FAILED  \n");
		 }
		 xil_printf("\nDMA status before transfer to device: %d",checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x34));
		 int status=checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x4);
		  while(status!=2)
		  {
		 	status=checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x4);
		 }

		 status=checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x34);
		  while(status!=2)
		 	 {
		 		 status=checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x34);
		 	}

		 	xil_printf("\nDMA status before transfer to device: %d",checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x34));
		 	xil_printf("\n printing FFT output on windowed");



		 //print out the fft of the windowed signal
		 for (int j=0;j<N;j++)
		 {
			 //sfft_spec[i][j]=abs(RX_PNTR[j]);
			// fputs(abs(RX_PNTR[j]), fptr);
			// fputs(",", fptr);

			 //text_temp=cabs(RX_PNTR[j]);
			 sprintf(text_temp,"%0.4f\n",cabs(RX_PNTR[j]));
			 writeFile(fptr, strlen(text_temp), text_temp);
			 //dataPntr = dataPntr+8;
			 printf("\n PL output: %f + I%f \n",crealf(RX_PNTR[j]),cimagf(RX_PNTR[j]));
			 printf("abs %f",cabs(RX_PNTR[j]));

		 }
		 //write data to the SD card
		 //writeFile(fptr, strlen(text_temp), text_temp);
		 //dataPntr = (char *)dataBuffer;


	 }



	  /*Xil_DCacheFlushRange(pFFT_input,(sizeof(float complex)*N));

	  Xil_DCacheFlushRange((UINTPTR)RX_PNTR,(sizeof(float complex)*N));

	  status_tf=XAxiDma_SimpleTransfer(&axiDma,pFFT_input,(sizeof(float complex)*N),XAXIDMA_DMA_TO_DEVICE);
	 if(status_tf!=XST_SUCCESS)
		 	{
		 		printf("WRITING DATA FROM DDR TO FIFO VIA DMA FAILED  \n");
		 	}
		 status_tf= XAxiDma_SimpleTransfer(&axiDma,(UINTPTR)RX_PNTR,(sizeof(float complex)*N),XAXIDMA_DEVICE_TO_DMA);
		 if(status_tf!=XST_SUCCESS)
		 	{
		 		printf("\n WRITING DATA FROM IP TO DMA FAILED  \n");
		 	}

		 xil_printf("\nDMA status before transfer to device: %d",checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x34));
		 int status=checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x4);
		 while(status!=2)
		 {
			 status=checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x4);
		 }

		 status=checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x34);
		 while(status!=2)
		 {
			 status=checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x34);
		 }

		 xil_printf("\nDMA status before transfer to device: %d",checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x34));
		 xil_printf("\n comparing software fft outputs and hardware");


		 //while(XAxiDma_Busy(&axiDma,XAXIDMA_DMA_TO_DEVICE));
		//while(XAxiDma_Busy(&axiDma,XAXIDMA_DEVICE_TO_DMA));
		 for (int j=0;j<N;j++)
		 {
			 printf("\n PS output: %f + I%f, PL output: %f + I%f \n",crealf(FFT_output_PS[j]),cimagf(FFT_output_PS[j]),crealf(RX_PNTR[j]),cimagf(RX_PNTR[j]));
			// float x=abs(crealf(RX_PNTR[j]));
			//printf("%f \n",abs(crealf(RX_PNTR[j])));
		 }*/
	 printf("\n Computation completed");

	 //eject the SD Card after writing to it
	 //xil_printf("WRITING TO SD Card...\n\r");
	 //writeFile(fptr, 11264, (u32)dataBuffer);
	 closeFile(fptr);
	 SD_Eject();
	 xil_printf("Safe to remove SD Card...\n\r");

    return 0;
}
