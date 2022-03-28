from gnuradio import gr
from gnuradio import blocks
from gnuradio import filter
from gnuradio.filter import firdes
from gnuradio import analog
import time
import osmosdr
import signal
import datetime

class Meteo_sat(gr.top_block):
    def __init__(self,name,frequence):
        gr.top_block.__init__(self, "Meteo sat")
        
        self.samp_rate = 1411200

        #Création of blocks 
        self.rational_resampler_xxx_0 = filter.rational_resampler_fff(
                interpolation=208,
                decimation=441,
                taps=None,
                fractional_bw=None)
        self.osmosdr_source_0 = osmosdr.source(
            args="numchan=" + str(1) + " " + ""
        )
        self.osmosdr_source_0.set_time_now(osmosdr.time_spec_t(time.time()), osmosdr.ALL_MBOARDS)
        self.osmosdr_source_0.set_sample_rate(self.samp_rate)
        self.osmosdr_source_0.set_center_freq(frequence, 0)
        self.osmosdr_source_0.set_freq_corr(0, 0)
        self.osmosdr_source_0.set_gain(40, 0)
        self.osmosdr_source_0.set_if_gain(20, 0)
        self.osmosdr_source_0.set_bb_gain(20, 0)
        self.osmosdr_source_0.set_antenna('', 0)
        self.osmosdr_source_0.set_bandwidth(0, 0)
        self.low_pass_filter_0 = filter.fir_filter_ccf(
            16,
            firdes.low_pass(
                1,
                self.samp_rate,
                110000,
                10000,
                firdes.WIN_HAMMING,
                6.76))
        self.blocks_wavfile_sink_0 = blocks.wavfile_sink(name, 1, 20800, 16)
        self.analog_wfm_rcv_0 = analog.wfm_rcv(
        	quad_rate=88200,
        	audio_decimation=2,
        )

        #Connection of blocks to each other
        self.connect((self.analog_wfm_rcv_0, 0), (self.rational_resampler_xxx_0, 0))
        self.connect((self.low_pass_filter_0, 0), (self.analog_wfm_rcv_0, 0))
        self.connect((self.osmosdr_source_0, 0), (self.low_pass_filter_0, 0))
        self.connect((self.rational_resampler_xxx_0, 0), (self.blocks_wavfile_sink_0, 0))


def main(nom,end_date,frequence):
    """
    Start by converting the date to an integer and then
    launch the acquisition of satellite data and wait
    that the date of end is reached
    return 0 if success
    """
    today=datetime.datetime.now()
    mois=int(end_date[0:2])
    jour=int(end_date[3:5])
    heure=int(end_date[6:8])
    minutes=int(end_date[9:11])
    annee=today.year
    dt = datetime.datetime(annee,mois,jour,heure,minutes)
    sleep=(dt-today).seconds
    
    Acquisition=Meteo_sat(nom,frequence)
    Acquisition.start()

    time.sleep(sleep)
    
    Acquisition.stop()
    Acquisition.wait()
    return 0


