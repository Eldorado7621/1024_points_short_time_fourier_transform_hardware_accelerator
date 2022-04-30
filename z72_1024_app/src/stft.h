
int lenStart(int window, int overlap,int length__of_array )
{
    int n_start=((length__of_array-1)/(window-overlap))+1;

    return n_start;
}


//functions creates the starting index of the window for each computation
int * win_start_index(int window,int overlap,int len_data)
{

    int ind=0;
    static int starts_win[11]; //C++ does not advocate to return the address of a local variable to outside of the function
    for (int i=0;i<=len_data;i=i+(window-overlap))
    {
        starts_win[ind]=i;
        ind++;
    }
    return starts_win;

}

/*int * create_windowed_signal(int Starts_win_ind, int window, float complex *ts_data)
{

	//get the windowed data
	static float complex windowed_signal[sizeof(*ts_data)]={0};
    int window_limit=Starts_win_ind+window;
    if(window_limit>sizeof(*ts_data)) //check if window limit is greater than the highest sample
    {
        window_limit=sizeof(*ts_data);
    }
    //extract from the original time series signal using the window
    for(int isg=Starts_win_ind;isg<window_limit;isg++)
    {
        windowed_signal[isg]=*(ts_data+1);
        xil_printf("\nwindow %d %d ",isg,windowed_signal[isg]);

    }

    return windowed_signal;


}
*/
int ln=1024;

float complex * create_windowed_signal(int Starts_win_ind, int window, float complex * ts_data,int length_of_array)
{

	//get the windowed data
	static float complex windowed_signal[1024]={0.0+I*0.0};
	for(int i=0;i<length_of_array;i++)
	{
		windowed_signal[i]=0.0+I*0.0;
	}
    int window_limit=Starts_win_ind+window;
    if(window_limit>length_of_array) //check if window limit is greater than the highest sample
    {
        window_limit=length_of_array;
    }
    xil_printf("\n start window %d",Starts_win_ind);

    //extract from the original time series signal using the window
    for(int isg=Starts_win_ind;isg<window_limit;isg++)
    {
        windowed_signal[isg]=*(ts_data+isg);
        xil_printf("\nwindow %d %d ",isg,windowed_signal[isg]);

    }
    //xil_printf("\nwindow 5 %d ",windowed_signal[5]);


    return windowed_signal;


}






