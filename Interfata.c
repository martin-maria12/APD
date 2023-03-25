#include <utility.h>
#include <ansi_c.h>
#include <advanlys.h>
#include <formatio.h>
#include <cvirte.h>		
#include <userint.h>
#include "Interfata.h"

// Constants
#define SAMPLE_RATE		0
#define NPOINTS			1

//==============================================================================
// Global variables
int waveInfo[2]; //waveInfo[0] = sampleRate
				 //waveInfo[1] = number of elements
double sampleRate = 0.0;
int npoints = 0, is_plot1 = -1, is_plot2 = -1;
double *waveData = 0, *filtru;
double *anvelope = 0;
//ordinul filtrului
int filterOrder = 1;
//frecventa de taiere jos
double lowerCutOff = 10;
//frecventa de taiere sus
double upperCutOff = 50;

int start = 0;
int stop = 0;

int indx = 0;

int frecvPanel = 0;
static int panelHandle;

int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, "Interfata.uir", PANEL)) < 0)
		return -1;
	if ((frecvPanel = LoadPanel (0, "Interfata.uir", FR_PANEL)) < 0)
		return -1;
	DisplayPanel (panelHandle);
	RunUserInterface ();
	DiscardPanel (panelHandle);
	return 0;
}

int CVICALLBACK OnLoadButtonCB (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	double min = 0.0;
	double max = 0.0;
	int MaxIndex = 0;
	int MinIndex = 0;
	double mean = 0.0;
	double mediana = 0.0;
	double intersectii = 0.0;
	double m = 0.0;
	double suma = 0.0;
		
	static ssize_t hist[101];
    static double axis[101];
	
	switch (event)
	{
		case EVENT_COMMIT:
			
			//executa script python pentru conversia unui fisierului .wav in .txt
			LaunchExecutable("python main.py");
			
			//astept sa fie generate cele doua fisiere (modificati timpul daca este necesar
			Delay(4);
			
			//incarc informatiile privind rata de esantionare si numarul de valori
			FileToArray("wafeInfo.txt", waveInfo, VAL_INTEGER, 2, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_COLUMNS, VAL_ASCII);
			sampleRate = waveInfo[SAMPLE_RATE];
			npoints = waveInfo[NPOINTS];
			
			//alocare memorie pentru numarul de puncte
			waveData = (double *) calloc(npoints, sizeof(double));
			anvelope = (double *) calloc(npoints,sizeof(double));
			
			//incarcare din fisierul .txt in memorie (vector)
			FileToArray("waveData.txt", waveData, VAL_DOUBLE, npoints, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_COLUMNS, VAL_ASCII);
			FileToArray("anvelopeAmplitude.txt", anvelope, VAL_DOUBLE, npoints, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_COLUMNS, VAL_ASCII);
			
			//afisare pe grapf
			DeleteGraphPlot(panel, PANEL_GRAPH_RAW_DATA, -1, VAL_IMMEDIATE_DRAW);
			is_plot1 = PlotY(panel, PANEL_GRAPH_RAW_DATA, waveData, npoints, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_DK_CYAN);
			//afisare anvelopa
			PlotY(panel, PANEL_GRAPH_RAW_DATA, anvelope, npoints, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS,VAL_BLUE);
			
			//Calcul maxim, minim, medie, mediana, dispersie, intersectii
			MaxMin1D(waveData, npoints, &max, &MaxIndex, &min, &MinIndex);
			Mean(waveData, npoints, &mean);
			SetCtrlVal(panel, PANEL_IDC_MIN, min);
			SetCtrlVal(panel, PANEL_IDC_MAX, max);
			SetCtrlVal(panel, PANEL_IDC_MEAN, mean);
						
			double* SortedWaveData = 0;
			SortedWaveData = (double *) calloc(npoints, sizeof(double));
			
			for(int i=1;i<npoints;i++){
				if(waveData[i-1] * waveData[i] < 0.0)
					intersectii++;
				SortedWaveData[i] = waveData[i];
				suma = suma + (waveData[i] - mean) * (waveData[i] - mean);
			}
						
			//media=suma/npoints;
			m = suma/npoints;
			
			SetCtrlVal(panel, PANEL_IDC_DISPERSIE, sqrt(m));
			SetCtrlVal(panel, PANEL_IDC_ZEROS, intersectii);
			SetCtrlVal(panel, PANEL_IDC_MIN_INDEX, MinIndex);
			SetCtrlVal(panel, PANEL_IDC_MAX_INDEX, MaxIndex);
			
			Sort(SortedWaveData,npoints,ANALYSIS_SORT_ASCENDING,SortedWaveData);
			
			mediana = SortedWaveData[npoints/2];
			SetCtrlVal(panel, PANEL_IDC_MEDIANA, mediana);
			
			//Histograma
			
			Histogram(waveData,npoints,min,max,hist,axis,1+3.3*log(npoints));
			PlotXY (panel, PANEL_HIST_GRAPH, axis,  hist,1+3.3*log(npoints), VAL_DOUBLE, VAL_SSIZE_T, VAL_VERTICAL_BAR, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_YELLOW);
			
			
			break;
	}
	return 0;
}


int CVICALLBACK OnPanel (int panel, int event, void *callbackData,
						 int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
			
		case EVENT_LOST_FOCUS:

			break;
			
		case EVENT_CLOSE:
			if(waveData != NULL)
				free(waveData);
			if(filtru !=NULL)
				free(filtru);
			QuitUserInterface(0);
			break;
	}
	return 0;
}

int CVICALLBACK ON_PREV (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		int tip_filtrare, stop = 1, start, img;
		char buffer[256];
		
		case EVENT_COMMIT:
			GetCtrlVal(panel, PANEL_STOP, &stop);
			GetCtrlVal(panel, PANEL_START, &start);
			GetCtrlVal(panel, PANEL_FILTRU, &tip_filtrare);   
			
			if(start >= 1)
			{
				SetCtrlVal(panel, PANEL_STOP, stop-1);
				SetCtrlVal(panel, PANEL_START, start-1);
				if(is_plot1 != -1)
				{
					SetAxisRange (panel, PANEL_GRAPH_RAW_DATA, VAL_MANUAL,  (start-1)*waveInfo[0], (stop-1)*waveInfo[0], VAL_AUTOSCALE, 1, 2);
					sprintf(buffer,"Photos\\Intervale\\Raw%d-%d.jpg",(start-1),(stop-1));
					GetCtrlDisplayBitmap(panel, PANEL_GRAPH_RAW_DATA, 1, &img);
					SaveBitmapToJPEGFile(img, buffer, JPEG_DCTFAST, 100);
					DiscardBitmap(img);
				}
				if(is_plot2!=-1)
				{
					SetAxisRange (panel, PANEL_GRAPH_FILTERED_DATA, VAL_MANUAL,  (start-1)*waveInfo[0], (stop-1)*waveInfo[0], VAL_AUTOSCALE, 1, 2);
					
					if(tip_filtrare == 0)
						sprintf(buffer,"Photos\\Intervale\\Median%d-%d.jpg",(start-1),(stop-1));  		
					else
						sprintf(buffer,"Photos\\Intervale\\Alpha%d-%d.jpg",(start-1),(stop-1));
					GetCtrlDisplayBitmap(panel, PANEL_GRAPH_FILTERED_DATA, 1, &img);
					SaveBitmapToJPEGFile(img, buffer, JPEG_DCTFAST, 100);
					DiscardBitmap(img);
				}
			}
			break;
	}
	return 0;
}

int CVICALLBACK ON_NEXT (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		int tip_filtrare, stop = 1, start, img;
		char buffer[256];
		
		case EVENT_COMMIT:
			GetCtrlVal(panel,PANEL_STOP,&stop);
			GetCtrlVal(panel, PANEL_START, &start);
			GetCtrlVal(panel, PANEL_FILTRU, &tip_filtrare);  
			
			if(stop < 7)
			{
				SetCtrlVal(panel, PANEL_STOP, stop+1);
				SetCtrlVal(panel, PANEL_START, start+1);
				if(is_plot1!=-1)
				{
					SetAxisRange (panel, PANEL_GRAPH_RAW_DATA, VAL_MANUAL,  (1+start)*waveInfo[0], (1+stop)*waveInfo[0], VAL_AUTOSCALE, 1, 2);
					sprintf(buffer,"Photos\\Intervale\\Raw%d-%d.jpg",(start+1),(1+stop));
					GetCtrlDisplayBitmap(panel, PANEL_GRAPH_RAW_DATA, 1, &img);
					SaveBitmapToJPEGFile(img, buffer, JPEG_DCTFAST, 100);
					DiscardBitmap(img);

				}
				
				if(is_plot2 != -1)
				{
					SetAxisRange (panel, PANEL_GRAPH_FILTERED_DATA, VAL_MANUAL,  (1+start)*waveInfo[0], (1+stop)*waveInfo[0], VAL_AUTOSCALE, 1, 2);
					if(tip_filtrare == 0)
						sprintf(buffer,"Photos\\Intervale\\Median%d-%d.jpg",(start+1),(1+stop));  
					else
						sprintf(buffer,"Photos\\Intervale\\Alpha%d-%d.jpg",(start+1),(1+stop));
					GetCtrlDisplayBitmap(panel, PANEL_GRAPH_FILTERED_DATA, 1, &img);
					SaveBitmapToJPEGFile(img, buffer, JPEG_DCTFAST, 100);
					DiscardBitmap(img);
				}
			}
			break;
		}
	return 0;
}

double * ordin1(double array [], int npoints, double alpha)
{
	int i;
	filtru = (double *) calloc(npoints, sizeof(double));
	filtru[0] = array[0];
	for(i = 1; i < npoints; i++)
	{
		filtru[i] = (1 - alpha) * filtru[i-1] + alpha * array[i];
	}
	return filtru;
}

int CVICALLBACK Filtru (int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2)
{
	double alpha;
	int val = 0;
	int img;
	char buffer[256];
	 
	switch (event)
	{
		case EVENT_COMMIT:
			
			GetCtrlVal(panel, PANEL_FILTRU, &val);
			GetCtrlVal(panel,PANEL_ALPHA_SET,&alpha);
			
			DeleteGraphPlot(panel,PANEL_GRAPH_FILTERED_DATA,-1,VAL_IMMEDIATE_DRAW);
				
			if(val == 1)
			{
				DeleteGraphPlot(panel,PANEL_GRAPH_FILTERED_DATA,-1,VAL_IMMEDIATE_DRAW);
				
				if(filtru != NULL)
					free(filtru);
				
				filtru = (double *) calloc(npoints, sizeof(double));
				
				if(is_plot1 != -1)
				{
					filtru = ordin1(waveData,npoints,alpha);
					is_plot2 = PlotY(panel,PANEL_GRAPH_FILTERED_DATA, filtru,npoints-1, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_GREEN);
					GetCtrlDisplayBitmap(panel,PANEL_GRAPH_FILTERED_DATA, 1, &img);
					sprintf(buffer, "Photos\\Alpha.jpg");
					SaveBitmapToJPEGFile(img, buffer, JPEG_DCTFAST, 100);
					DiscardBitmap(img);
				}
				
			}
			else
				if(val == 0)
			{
				
				DeleteGraphPlot(panel,PANEL_GRAPH_FILTERED_DATA,-1,VAL_IMMEDIATE_DRAW);
				
				if(filtru != NULL)
					free(filtru);
				filtru = (double *) calloc(npoints, sizeof(double));
				MedianFilter(waveData,npoints,32,16,filtru);
				is_plot2 = PlotY(panel,PANEL_GRAPH_FILTERED_DATA, filtru,npoints, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_MAGENTA);
				GetCtrlDisplayBitmap(panel,PANEL_GRAPH_FILTERED_DATA, 1, &img);
				sprintf(buffer, "Photos\\Median.jpg");
				SaveBitmapToJPEGFile(img, buffer, JPEG_DCTFAST, 100);
				DiscardBitmap(img);
			}

			break;
	}
	return 0;
}

int CVICALLBACK OnFrPanel (int panel, int event, void *callbackData,
						   int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			QuitUserInterface(0);
			break;
	}
	return 0;
}

int CVICALLBACK OnSwitchPanels (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(panel == panelHandle)
			{
				SetCtrlVal(frecvPanel, FR_PANEL_SWITCHPANELS, 1);
				DisplayPanel(frecvPanel);
				HidePanel(panel);
			}
			else
			{
				SetCtrlVal(panelHandle, PANEL_SWITCHPANELS, 0);
				DisplayPanel(panelHandle);
				HidePanel(panel);
			}
			break;
	}
	return 0;
}

int CVICALLBACK OnLoadFRPANEL (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		
		case EVENT_COMMIT:
			//Al doilea panel
			
			//executa script python pentru conversia unui fisierului .wav in .txt
			LaunchExecutable("python main.py");
			
			//astept sa fie generate cele doua fisiere (modificati timpul daca este necesar
			Delay(4);
			
			//incarc informatiile privind rata de esantionare si numarul de valori
			FileToArray("wafeInfo.txt", waveInfo, VAL_INTEGER, 2, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_COLUMNS, VAL_ASCII);
			sampleRate = waveInfo[SAMPLE_RATE];
			npoints = waveInfo[NPOINTS];
			
			//alocare memorie pentru numarul de puncte
			waveData = (double *) calloc(npoints, sizeof(double));
			
			//incarcare din fisierul .txt in memorie (vector)
			FileToArray("waveData.txt", waveData, VAL_DOUBLE, npoints, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_COLUMNS, VAL_ASCII);
			
			//afisare pe graf de semnal original
			DeleteGraphPlot(panel, FR_PANEL_Original_Signal_Graph, -1, VAL_IMMEDIATE_DRAW);
			PlotY(panel, FR_PANEL_Original_Signal_Graph, waveData, npoints, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_DK_CYAN);
			
			break;
	}
	return 0;
}


int CVICALLBACK Fereastra (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	int fereastra = 0;
	int N;
	//
	WindowConst winConst;
	
	double *copieWaveData;
	 
	switch (event)
	{
		case EVENT_COMMIT:
					
			GetCtrlVal(panel, FR_PANEL_TIPFEREASTRA, &fereastra);
			GetCtrlVal(panel, FR_PANEL_NOfPoints, &N);
			
			//vector ce contine spectrul semnalului convertit la volti
			double convertedSpectrum[N/2];
			double convertedSpectrum2[N/2];
			double convertedSpectrum3[N/2];
			//frecventa estimata pentru spectrul de putere (maxim) din vectorul autoSpectrum
			double powerPeak = 0.0;
			double powerPeak2 = 0.0;
			double powerPeak3 = 0.0;
			//valoarea maxima din spectru de putere (din autoSpectrum)
			double freqPeak = 0.0;
			double freqPeak2 = 0.0;
			double freqPeak3 = 0.0;
			//variabila ce reprezinta pasul in domeniul frecventei
			double df = 0.0;
			//voltage signal - descriere a semnalului achizitionat
			char unit[32] = "V";
			//spectrul de putere cu un numar de valori egal cu jumatate din dimensiunea bufferuluide intrare
			double autoSpectrum[N/2];
			double autoSpectrum2[N/2];
			double autoSpectrum3[N/2];			
			
			int imgHandle;
			char buffer[256];
						
			//alocare memorie pentru numarul de puncte
			copieWaveData = (double *) calloc(N, sizeof(double));
			
			for(int i=0;i<N;i++)
				copieWaveData[i] = waveData[i];
			
			double hCoef[45];
			double filteredSignal[45+N-1];
			double filteredSignal2[N];
			
			//fereastra Hamming
			if(fereastra == 0){
			
				SetCtrlAttribute(panel, FR_PANEL_TIMER, ATTR_INTERVAL, 1.0/N);
				//afisare pe graf de semnal original cu ferestruire
				ScaledWindowEx(copieWaveData, N, HAMMING, 0, &winConst);	
				DeleteGraphPlot(panel, FR_PANEL_WINDOWED_SIGNAL, -1, VAL_IMMEDIATE_DRAW);
				PlotY(panel, FR_PANEL_WINDOWED_SIGNAL, copieWaveData, N, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_GREEN);
				
				GetCtrlDisplayBitmap(panel,FR_PANEL_WINDOWED_SIGNAL, 1, &imgHandle);
				sprintf(buffer, "PhotosEtapa2\\FerestruireHamming.jpg");
				SaveBitmapToJPEGFile(imgHandle, buffer, JPEG_DCTFAST, 100);
				DiscardBitmap(imgHandle);
				
				//determinare spectru de putere
					//partea pozitiva a spectrului scalat de putere pentru un semnal esantionat
				AutoPowerSpectrum(copieWaveData, N, 1./N, autoSpectrum, &df);
					//calculeaza puterea si frecventa corespunzatoare varfului din spectrul semnalului  
				PowerFrequencyEstimate(autoSpectrum, N/2, -1, winConst, df, 7, &freqPeak, &powerPeak);
					//afisez pe interfata valorile determinate
				SetCtrlVal(panel, FR_PANEL_IDC_FREQ_PEAK, freqPeak);
				SetCtrlVal(panel, FR_PANEL_IDC_POWER_PEAK, powerPeak);
			    	//Functia converteste spectrul de intrare (care este puterea, amplitudinea sau amplificarea) 
					//în formate alternative (linear, logarithmic, dB) ce permit o reprezentare grafica mai convenabila.
				SpectrumUnitConversion(autoSpectrum, N/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS2, df, winConst, convertedSpectrum, unit);
					//sterg graph-ul unde urmeaza sa plotez spectrul semnalului
				DeleteGraphPlot (panel, FR_PANEL_SPECTRUM_NEFILTRAT, -1, VAL_IMMEDIATE_DRAW);	
					//plotez spectrul semnalului
			    PlotWaveform( panel, FR_PANEL_SPECTRUM_NEFILTRAT, convertedSpectrum, N/2 ,VAL_DOUBLE, 1.0, 0.0, 0.0, df,
			                                    VAL_FAT_LINE, VAL_EMPTY_SQUARE, VAL_SOLID,  VAL_CONNECTED_POINTS, VAL_GREEN);
				
				GetCtrlDisplayBitmap(panel,FR_PANEL_SPECTRUM_NEFILTRAT, 1, &imgHandle);
				sprintf(buffer, "PhotosEtapa2\\FerestruireHammingSpectru.jpg");
				SaveBitmapToJPEGFile(imgHandle, buffer, JPEG_DCTFAST, 100);
				DiscardBitmap(imgHandle);	
					
				
				//filtru KSR_HPF
				Ksr_HPF(sampleRate, 1200, 45, hCoef, 0.0);
				Convolve(hCoef, 45, copieWaveData, N, filteredSignal);
				DeleteGraphPlot(panel, FR_PANEL_KRS_HPF_GRAPH, -1, VAL_IMMEDIATE_DRAW);
				PlotY(panel, FR_PANEL_KRS_HPF_GRAPH, filteredSignal, N, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_MAGENTA);
				
				GetCtrlDisplayBitmap(panel,FR_PANEL_KRS_HPF_GRAPH, 1, &imgHandle);
				sprintf(buffer, "PhotosEtapa2\\FerestruireHammingFiltruKSR_HPF.jpg");
				SaveBitmapToJPEGFile(imgHandle, buffer, JPEG_DCTFAST, 100);
				DiscardBitmap(imgHandle);
				
					//determinare spectru de putere
					//partea pozitiva a spectrului scalat de putere pentru un semnal esantionat
				AutoPowerSpectrum(filteredSignal, N, 1./N, autoSpectrum2, &df);
					//calculeaza puterea si frecventa corespunzatoare varfului din spectrul semnalului  
				PowerFrequencyEstimate(autoSpectrum2, N/2, -1, winConst, df, 7, &freqPeak2, &powerPeak2);
					//afisez pe interfata valorile determinate
				SetCtrlVal(panel, FR_PANEL_IDC_FREQ_PEAK_2, freqPeak2);
				SetCtrlVal(panel, FR_PANEL_IDC_POWER_PEAK_2, powerPeak2);
			    	//Functia converteste spectrul de intrare (care este puterea, amplitudinea sau amplificarea) 
					//în formate alternative (linear, logarithmic, dB) ce permit o reprezentare grafica mai convenabila.
				SpectrumUnitConversion(autoSpectrum2, N/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS2, df, winConst, convertedSpectrum2, unit);
					//sterg graph-ul unde urmeaza sa plotez spectrul semnalului
				DeleteGraphPlot (panel, FR_PANEL_SPECTRUM_KRS_GRAPH, -1, VAL_IMMEDIATE_DRAW);	
					//plotez spectrul semnalului
			    PlotWaveform( panel, FR_PANEL_SPECTRUM_KRS_GRAPH, convertedSpectrum2, N/2 ,VAL_DOUBLE, 1.0, 0.0, 0.0, df,
			                                    VAL_FAT_LINE, VAL_EMPTY_SQUARE, VAL_SOLID,  VAL_CONNECTED_POINTS, VAL_MAGENTA);
				
				GetCtrlDisplayBitmap(panel,FR_PANEL_SPECTRUM_KRS_GRAPH, 1, &imgHandle);
				sprintf(buffer, "PhotosEtapa2\\FerestruireHammingFiltruKSR_HPFSpectru.jpg");
				SaveBitmapToJPEGFile(imgHandle, buffer, JPEG_DCTFAST, 100);
				DiscardBitmap(imgHandle);
			
			
					//filtru Butterworth trece sus
				Bw_HPF(copieWaveData, N, sampleRate, upperCutOff, filterOrder, filteredSignal2);
				DeleteGraphPlot(panel, FR_PANEL_BUTTERWORTH_GRAPH, -1, VAL_IMMEDIATE_DRAW);
				PlotY(panel, FR_PANEL_BUTTERWORTH_GRAPH, filteredSignal2,N,VAL_DOUBLE,VAL_THIN_LINE,VAL_EMPTY_SQUARE,VAL_SOLID,1,VAL_CYAN);  
				
				GetCtrlDisplayBitmap(panel,FR_PANEL_BUTTERWORTH_GRAPH, 1, &imgHandle);
				sprintf(buffer, "PhotosEtapa2\\FerestruireHammingFiltruButterworth.jpg");
				SaveBitmapToJPEGFile(imgHandle, buffer, JPEG_DCTFAST, 100);
				DiscardBitmap(imgHandle);
				
			//determinare spectru de putere
					//partea pozitiva a spectrului scalat de putere pentru un semnal esantionat
				AutoPowerSpectrum(filteredSignal2, N, 1./N, autoSpectrum3, &df);
					//calculeaza puterea si frecventa corespunzatoare varfului din spectrul semnalului  
				PowerFrequencyEstimate(autoSpectrum3, N/2, -1, winConst, df, 7, &freqPeak3, &powerPeak3);
					//afisez pe interfata valorile determinate
				SetCtrlVal(panel, FR_PANEL_IDC_FREQ_PEAK_3, freqPeak3);
				SetCtrlVal(panel, FR_PANEL_IDC_POWER_PEAK_3, powerPeak3);
			    	//Functia converteste spectrul de intrare (care este puterea, amplitudinea sau amplificarea) 
					//în formate alternative (linear, logarithmic, dB) ce permit o reprezentare grafica mai convenabila.
				SpectrumUnitConversion(autoSpectrum3, N/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS2, df, winConst, convertedSpectrum3, unit);
				//sterg graph-ul unde urmeaza sa plotez spectrul semnalului
				DeleteGraphPlot (panel, FR_PANEL_SPECTRUM_BUTTERWORTH, -1, VAL_IMMEDIATE_DRAW);	
					//plotez spectrul semnalului
			    PlotWaveform( panel, FR_PANEL_SPECTRUM_BUTTERWORTH, convertedSpectrum3, N/2 ,VAL_DOUBLE, 1.0, 0.0, 0.0, df,
			                                    VAL_FAT_LINE, VAL_EMPTY_SQUARE, VAL_SOLID,  VAL_CONNECTED_POINTS, VAL_CYAN);
				
				GetCtrlDisplayBitmap(panel,FR_PANEL_SPECTRUM_BUTTERWORTH, 1, &imgHandle);
				sprintf(buffer, "PhotosEtapa2\\FerestruireHammingFiltruButterworthSpectru.jpg");
				SaveBitmapToJPEGFile(imgHandle, buffer, JPEG_DCTFAST, 100);
				DiscardBitmap(imgHandle);
				
				SetCtrlAttribute(panel, FR_PANEL_TIMER, ATTR_ENABLED,1);		
			}
			//fereastra Welch
			else
				if(fereastra == 1)
				{
					SetCtrlAttribute(panel, FR_PANEL_TIMER, ATTR_INTERVAL, 1.0/N);	
					//afisare pe graf de semnal original cu ferestruire
					ScaledWindowEx(copieWaveData, N, WELCH, 0, &winConst);	
					DeleteGraphPlot(panel, FR_PANEL_WINDOWED_SIGNAL, -1, VAL_IMMEDIATE_DRAW);
					PlotY(panel, FR_PANEL_WINDOWED_SIGNAL, copieWaveData, N, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_YELLOW);
					
					GetCtrlDisplayBitmap(panel,FR_PANEL_WINDOWED_SIGNAL, 1, &imgHandle);
					sprintf(buffer, "PhotosEtapa2\\FerestruireWelch.jpg");
					SaveBitmapToJPEGFile(imgHandle, buffer, JPEG_DCTFAST, 100);
					DiscardBitmap(imgHandle);
					
						//determinare spectru de putere
						//partea pozitiva a spectrului scalat de putere pentru un semnal esantionat
					AutoPowerSpectrum(copieWaveData, N, 1./N, autoSpectrum, &df);
						//calculeaza puterea si frecventa corespunzatoare varfului din spectrul semnalului  
					PowerFrequencyEstimate(autoSpectrum, N/2, -1, winConst, df, 7, &freqPeak, &powerPeak);
						//afisez pe interfata valorile determinate
					SetCtrlVal(panel, FR_PANEL_IDC_FREQ_PEAK, freqPeak);
					SetCtrlVal(panel, FR_PANEL_IDC_POWER_PEAK, powerPeak);
				    	//Functia converteste spectrul de intrare (care este puterea, amplitudinea sau amplificarea) 
						//în formate alternative (linear, logarithmic, dB) ce permit o reprezentare grafica mai convenabila.
					SpectrumUnitConversion(autoSpectrum, N/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS2, df, winConst, convertedSpectrum, unit);
					
						//sterg graph-ul unde urmeaza sa plotez spectrul semnalului
					DeleteGraphPlot (panel, FR_PANEL_SPECTRUM_NEFILTRAT, -1, VAL_IMMEDIATE_DRAW);	
						//plotez spectrul semnalului
				    PlotWaveform( panel, FR_PANEL_SPECTRUM_NEFILTRAT, convertedSpectrum, N/2 ,VAL_DOUBLE, 1.0, 0.0, 0.0, df,
				                                    VAL_FAT_LINE, VAL_EMPTY_SQUARE, VAL_SOLID,  VAL_CONNECTED_POINTS, VAL_YELLOW);
					
					GetCtrlDisplayBitmap(panel,FR_PANEL_SPECTRUM_NEFILTRAT, 1, &imgHandle);
					sprintf(buffer, "PhotosEtapa2\\FerestruireWelchSpectru.jpg");
					SaveBitmapToJPEGFile(imgHandle, buffer, JPEG_DCTFAST, 100);
					DiscardBitmap(imgHandle);	
					
					//filtru KSR_HPF
					Ksr_HPF(sampleRate, 1200, 45, hCoef, 0.0);
					Convolve(hCoef, 45, copieWaveData, N, filteredSignal);
					DeleteGraphPlot(panel, FR_PANEL_KRS_HPF_GRAPH, -1, VAL_IMMEDIATE_DRAW);
					PlotY(panel, FR_PANEL_KRS_HPF_GRAPH, filteredSignal, N, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_WHITE);
					
					GetCtrlDisplayBitmap(panel,FR_PANEL_KRS_HPF_GRAPH, 1, &imgHandle);
					sprintf(buffer, "PhotosEtapa2\\FerestruireWelchFiltruKSR_HPF.jpg");
					SaveBitmapToJPEGFile(imgHandle, buffer, JPEG_DCTFAST, 100);
					DiscardBitmap(imgHandle);
					
						//determinare spectru de putere
						//partea pozitiva a spectrului scalat de putere pentru un semnal esantionat
					AutoPowerSpectrum(filteredSignal, N, 1./N, autoSpectrum2, &df);
						//calculeaza puterea si frecventa corespunzatoare varfului din spectrul semnalului  
					PowerFrequencyEstimate(autoSpectrum2, N/2, -1, winConst, df, 7, &freqPeak2, &powerPeak2);
						//afisez pe interfata valorile determinate
					SetCtrlVal(panel, FR_PANEL_IDC_FREQ_PEAK_2, freqPeak2);
					SetCtrlVal(panel, FR_PANEL_IDC_POWER_PEAK_2, powerPeak2);
				    	//Functia converteste spectrul de intrare (care este puterea, amplitudinea sau amplificarea) 
						//în formate alternative (linear, logarithmic, dB) ce permit o reprezentare grafica mai convenabila.
					SpectrumUnitConversion(autoSpectrum2, N/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS2, df, winConst, convertedSpectrum2, unit);
					//sterg graph-ul unde urmeaza sa plotez spectrul semnalului
					DeleteGraphPlot (panel, FR_PANEL_SPECTRUM_KRS_GRAPH, -1, VAL_IMMEDIATE_DRAW);	
						//plotez spectrul semnalului
				    PlotWaveform( panel, FR_PANEL_SPECTRUM_KRS_GRAPH, convertedSpectrum2, N/2 ,VAL_DOUBLE, 1.0, 0.0, 0.0, df,
				                                    VAL_FAT_LINE, VAL_EMPTY_SQUARE, VAL_SOLID,  VAL_CONNECTED_POINTS,  VAL_WHITE);
					
					GetCtrlDisplayBitmap(panel,FR_PANEL_SPECTRUM_KRS_GRAPH, 1, &imgHandle);
					sprintf(buffer, "PhotosEtapa2\\FerestruireWelchFiltruKSR_HPFSpectru.jpg");
					SaveBitmapToJPEGFile(imgHandle, buffer, JPEG_DCTFAST, 100);
					DiscardBitmap(imgHandle);
					

					//filtru Butterworth trece sus
					Bw_HPF(copieWaveData, N, sampleRate, upperCutOff, filterOrder, filteredSignal2);
					DeleteGraphPlot(panel, FR_PANEL_BUTTERWORTH_GRAPH, -1, VAL_IMMEDIATE_DRAW);
					PlotY(panel, FR_PANEL_BUTTERWORTH_GRAPH, filteredSignal2,N,VAL_DOUBLE,VAL_THIN_LINE,VAL_EMPTY_SQUARE,VAL_SOLID,1,VAL_MAGENTA);
					
					GetCtrlDisplayBitmap(panel,FR_PANEL_BUTTERWORTH_GRAPH, 1, &imgHandle);
					sprintf(buffer, "PhotosEtapa2\\FerestruireWelchFiltruButterworth.jpg");
					SaveBitmapToJPEGFile(imgHandle, buffer, JPEG_DCTFAST, 100);
					DiscardBitmap(imgHandle);
					
					//determinare spectru de putere
						//partea pozitiva a spectrului scalat de putere pentru un semnal esantionat
					AutoPowerSpectrum(filteredSignal2, N, 1./N, autoSpectrum3, &df);
						//calculeaza puterea si frecventa corespunzatoare varfului din spectrul semnalului  
					PowerFrequencyEstimate(autoSpectrum3, N/2, -1, winConst, df, 7, &freqPeak3, &powerPeak3);
						//afisez pe interfata valorile determinate
					SetCtrlVal(panel, FR_PANEL_IDC_FREQ_PEAK_3, freqPeak3);
					SetCtrlVal(panel, FR_PANEL_IDC_POWER_PEAK_3, powerPeak3);
				    	//Functia converteste spectrul de intrare (care este puterea, amplitudinea sau amplificarea) 
						//în formate alternative (linear, logarithmic, dB) ce permit o reprezentare grafica mai convenabila.
					SpectrumUnitConversion(autoSpectrum3, N/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS2, df, winConst, convertedSpectrum3, unit);
					//sterg graph-ul unde urmeaza sa plotez spectrul semnalului
					DeleteGraphPlot (panel, FR_PANEL_SPECTRUM_BUTTERWORTH, -1, VAL_IMMEDIATE_DRAW);	
						//plotez spectrul semnalului
				    PlotWaveform( panel, FR_PANEL_SPECTRUM_BUTTERWORTH, convertedSpectrum3, N/2 ,VAL_DOUBLE, 1.0, 0.0, 0.0, df,
				                                    VAL_FAT_LINE, VAL_EMPTY_SQUARE, VAL_SOLID,  VAL_CONNECTED_POINTS, VAL_MAGENTA);
										
					GetCtrlDisplayBitmap(panel,FR_PANEL_SPECTRUM_BUTTERWORTH, 1, &imgHandle);
					sprintf(buffer, "PhotosEtapa2\\FerestruireWelchFiltruButterworthSpectru.jpg");
					SaveBitmapToJPEGFile(imgHandle, buffer, JPEG_DCTFAST, 100);
					DiscardBitmap(imgHandle);		
					
					SetCtrlAttribute(panel, FR_PANEL_TIMER, ATTR_ENABLED,1);
				}
			break;
	}
	return 0;
}

int CVICALLBACK OnUpperCutOffCB (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			//obtin frecventa de taiere sus
			GetCtrlVal(panel, control, &upperCutOff);
			break;
	}
	return 0;
}

int CVICALLBACK OnLowerCutOffCB (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			//obtin frecventa de taiere jos
			GetCtrlVal(panel, control, &lowerCutOff);
			break;
	}
	return 0;
}

int CVICALLBACK OnTimer (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	int N;
	
	switch (event)
	{
		case EVENT_COMMIT:
			
			GetCtrlVal(panel, FR_PANEL_NOfPoints, &N);
			
			indx = (indx < N-1)? (indx+1):0;
		break;
	}
	return 0;
}
