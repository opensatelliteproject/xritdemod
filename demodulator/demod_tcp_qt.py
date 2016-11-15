#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: LRIT Demodulator
# Author: Lucas Teske
# Description: https://github.com/racerxdl/open-satellite-project
# Generated: Tue Nov 15 17:17:34 2016
##################################################

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

from PyQt4 import Qt
from gnuradio import analog
from gnuradio import blocks
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio import qtgui
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from gnuradio.qtgui import Range, RangeWidget
from optparse import OptionParser
import osmosdr
import sip
import sys
import time


class demod_tcp_qt(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "LRIT Demodulator")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("LRIT Demodulator")
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "demod_tcp_qt")
        self.restoreGeometry(self.settings.value("geometry").toByteArray())

        ##################################################
        # Variables
        ##################################################
        self.symbol_rate = symbol_rate = 293883
        self.samp_rate = samp_rate = 10e6/10
        self.vgagain = vgagain = 15
        self.sps = sps = (samp_rate*1.0)/(symbol_rate*1.0)
        self.pll_alpha = pll_alpha = 0.00199
        self.mixgain = mixgain = 15
        self.lnagain = lnagain = 15
        self.clock_alpha = clock_alpha = 3.7e-3
        self.center_freq = center_freq = 1691e6

        ##################################################
        # Blocks
        ##################################################
        self._vgagain_range = Range(0, 15, 1, 15, 150)
        self._vgagain_win = RangeWidget(self._vgagain_range, self.set_vgagain, 'VGA Gain', "counter_slider", float)
        self.top_grid_layout.addWidget(self._vgagain_win, 4,1,1,1)
        self._pll_alpha_range = Range(0.00100, 0.10000, 0.00001, 0.00199, 150)
        self._pll_alpha_win = RangeWidget(self._pll_alpha_range, self.set_pll_alpha, 'PLL Alpha', "counter_slider", float)
        self.top_grid_layout.addWidget(self._pll_alpha_win, 6,1,1,1)
        self._mixgain_range = Range(0, 15, 1, 15, 150)
        self._mixgain_win = RangeWidget(self._mixgain_range, self.set_mixgain, 'Mixer Gain', "counter_slider", int)
        self.top_grid_layout.addWidget(self._mixgain_win, 3,1,1,1)
        self._lnagain_range = Range(0, 15, 1, 15, 150)
        self._lnagain_win = RangeWidget(self._lnagain_range, self.set_lnagain, 'LNA Gain', "counter_slider", int)
        self.top_grid_layout.addWidget(self._lnagain_win, 2,1,1,1)
        self._clock_alpha_range = Range(1e-3, 10e-3, 1e-5, 3.7e-3, 150)
        self._clock_alpha_win = RangeWidget(self._clock_alpha_range, self.set_clock_alpha, 'Clock Alpha', "counter_slider", float)
        self.top_grid_layout.addWidget(self._clock_alpha_win, 5,1,1,1)
        self.root_raised_cosine_filter_0 = filter.fir_filter_ccf(1, firdes.root_raised_cosine(
        	1, samp_rate, symbol_rate, 0.5, 361))
        self.qtgui_waterfall_sink_x_0 = qtgui.waterfall_sink_c(
        	8192, #size
        	firdes.WIN_BLACKMAN_hARRIS, #wintype
        	center_freq, #fc
        	samp_rate, #bw
        	"", #name
                1 #number of inputs
        )
        self.qtgui_waterfall_sink_x_0.set_update_time(0.05)
        self.qtgui_waterfall_sink_x_0.enable_grid(False)
        self.qtgui_waterfall_sink_x_0.enable_axis_labels(False)
        
        if not False:
          self.qtgui_waterfall_sink_x_0.disable_legend()
        
        if "complex" == "float" or "complex" == "msg_float":
          self.qtgui_waterfall_sink_x_0.set_plot_pos_half(not True)
        
        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [3, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]
        for i in xrange(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_0.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_0.set_line_alpha(i, alphas[i])
        
        self.qtgui_waterfall_sink_x_0.set_intensity_range(-50, -40)
        
        self._qtgui_waterfall_sink_x_0_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_0_win, 1,0,6,1)
        self.qtgui_number_sink_0 = qtgui.number_sink(
            gr.sizeof_float,
            0.3,
            qtgui.NUM_GRAPH_VERT,
            1
        )
        self.qtgui_number_sink_0.set_update_time(0.10)
        self.qtgui_number_sink_0.set_title('SNR')
        
        labels = [' ', '', '', '', '',
                  '', '', '', '', '']
        units = ['dB', '', '', '', '',
                 '', '', '', '', '']
        colors = [("black", "black"), ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black"),
                  ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black")]
        factor = [1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1]
        for i in xrange(1):
            self.qtgui_number_sink_0.set_min(i, 0)
            self.qtgui_number_sink_0.set_max(i, 10)
            self.qtgui_number_sink_0.set_color(i, colors[i][0], colors[i][1])
            if len(labels[i]) == 0:
                self.qtgui_number_sink_0.set_label(i, "Data {0}".format(i))
            else:
                self.qtgui_number_sink_0.set_label(i, labels[i])
            self.qtgui_number_sink_0.set_unit(i, units[i])
            self.qtgui_number_sink_0.set_factor(i, factor[i])
        
        self.qtgui_number_sink_0.enable_autoscale(False)
        self._qtgui_number_sink_0_win = sip.wrapinstance(self.qtgui_number_sink_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_number_sink_0_win, 0,2,7,1)
        self.qtgui_freq_sink_x_0 = qtgui.freq_sink_c(
        	8192, #size
        	firdes.WIN_BLACKMAN_hARRIS, #wintype
        	center_freq, #fc
        	samp_rate, #bw
        	'Input Signal', #name
        	1 #number of inputs
        )
        self.qtgui_freq_sink_x_0.set_update_time(0.05)
        self.qtgui_freq_sink_x_0.set_y_axis(-60, -40)
        self.qtgui_freq_sink_x_0.set_y_label('Relative Gain', 'dB')
        self.qtgui_freq_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_freq_sink_x_0.enable_autoscale(False)
        self.qtgui_freq_sink_x_0.enable_grid(True)
        self.qtgui_freq_sink_x_0.set_fft_average(0.1)
        self.qtgui_freq_sink_x_0.enable_axis_labels(False)
        self.qtgui_freq_sink_x_0.enable_control_panel(False)
        
        if not False:
          self.qtgui_freq_sink_x_0.disable_legend()
        
        if "complex" == "float" or "complex" == "msg_float":
          self.qtgui_freq_sink_x_0.set_plot_pos_half(not True)
        
        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1]
        colors = ["dark red", "red", "green", "black", "cyan",
                  "magenta", "yellow", "dark red", "dark green", "dark blue"]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]
        for i in xrange(1):
            if len(labels[i]) == 0:
                self.qtgui_freq_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_freq_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_freq_sink_x_0.set_line_width(i, widths[i])
            self.qtgui_freq_sink_x_0.set_line_color(i, colors[i])
            self.qtgui_freq_sink_x_0.set_line_alpha(i, alphas[i])
        
        self._qtgui_freq_sink_x_0_win = sip.wrapinstance(self.qtgui_freq_sink_x_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_freq_sink_x_0_win, 0,0,1,1)
        self.qtgui_const_sink_x_0 = qtgui.const_sink_c(
        	1024, #size
        	'Output Constellation', #name
        	1 #number of inputs
        )
        self.qtgui_const_sink_x_0.set_update_time(0.05)
        self.qtgui_const_sink_x_0.set_y_axis(-1, 1)
        self.qtgui_const_sink_x_0.set_x_axis(-1, 1)
        self.qtgui_const_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, "")
        self.qtgui_const_sink_x_0.enable_autoscale(False)
        self.qtgui_const_sink_x_0.enable_grid(True)
        self.qtgui_const_sink_x_0.enable_axis_labels(False)
        
        if not False:
          self.qtgui_const_sink_x_0.disable_legend()
        
        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1]
        colors = ["dark green", "red", "red", "red", "red",
                  "red", "red", "red", "red", "red"]
        styles = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        markers = [0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]
        for i in xrange(1):
            if len(labels[i]) == 0:
                self.qtgui_const_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_const_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_const_sink_x_0.set_line_width(i, widths[i])
            self.qtgui_const_sink_x_0.set_line_color(i, colors[i])
            self.qtgui_const_sink_x_0.set_line_style(i, styles[i])
            self.qtgui_const_sink_x_0.set_line_marker(i, markers[i])
            self.qtgui_const_sink_x_0.set_line_alpha(i, alphas[i])
        
        self._qtgui_const_sink_x_0_win = sip.wrapinstance(self.qtgui_const_sink_x_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_const_sink_x_0_win, 0,1,1,1)
        self.osmosdr_source_0 = osmosdr.source( args="numchan=" + str(1) + " " + 'airspy=0' )
        self.osmosdr_source_0.set_sample_rate(samp_rate*10)
        self.osmosdr_source_0.set_center_freq(center_freq, 0)
        self.osmosdr_source_0.set_freq_corr(0, 0)
        self.osmosdr_source_0.set_dc_offset_mode(1, 0)
        self.osmosdr_source_0.set_iq_balance_mode(1, 0)
        self.osmosdr_source_0.set_gain_mode(True, 0)
        self.osmosdr_source_0.set_gain(lnagain, 0)
        self.osmosdr_source_0.set_if_gain(vgagain, 0)
        self.osmosdr_source_0.set_bb_gain(mixgain, 0)
        self.osmosdr_source_0.set_antenna('', 0)
        self.osmosdr_source_0.set_bandwidth(0, 0)
          
        self.low_pass_filter_0 = filter.fir_filter_ccf(10, firdes.low_pass(
        	1, samp_rate*10, samp_rate/2, 100e3, firdes.WIN_HAMMING, 6.76))
        self.high_pass_filter_0 = filter.fir_filter_ccf(1, firdes.high_pass(
        	1, samp_rate, symbol_rate, 300e3, firdes.WIN_BLACKMAN, 6.76))
        self.digital_costas_loop_cc_0 = digital.costas_loop_cc(pll_alpha, 2, False)
        self.digital_clock_recovery_mm_xx_0 = digital.clock_recovery_mm_cc(sps, clock_alpha**2/4.0, 0.5, clock_alpha, 0.005)
        self.blocks_stream_to_vector_0 = blocks.stream_to_vector(gr.sizeof_char*1, 64)
        self.blocks_rms_xx_0_0 = blocks.rms_cf(0.0001)
        self.blocks_rms_xx_0 = blocks.rms_cf(0.0001)
        self.blocks_null_sink_0 = blocks.null_sink(gr.sizeof_char*64)
        self.blocks_nlog10_ff_0 = blocks.nlog10_ff(20, 1, 0)
        self.blocks_float_to_char_0 = blocks.float_to_char(1, 127)
        self.blocks_divide_xx_0 = blocks.divide_ff(1)
        self.blocks_complex_to_real_0 = blocks.complex_to_real(1)
        self.analog_agc_xx_0 = analog.agc_cc(100e-4, 0.5, 0.5)
        self.analog_agc_xx_0.set_max_gain(4000)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_agc_xx_0, 0), (self.high_pass_filter_0, 0))    
        self.connect((self.analog_agc_xx_0, 0), (self.qtgui_freq_sink_x_0, 0))    
        self.connect((self.analog_agc_xx_0, 0), (self.qtgui_waterfall_sink_x_0, 0))    
        self.connect((self.analog_agc_xx_0, 0), (self.root_raised_cosine_filter_0, 0))    
        self.connect((self.blocks_complex_to_real_0, 0), (self.blocks_float_to_char_0, 0))    
        self.connect((self.blocks_divide_xx_0, 0), (self.blocks_nlog10_ff_0, 0))    
        self.connect((self.blocks_float_to_char_0, 0), (self.blocks_stream_to_vector_0, 0))    
        self.connect((self.blocks_nlog10_ff_0, 0), (self.qtgui_number_sink_0, 0))    
        self.connect((self.blocks_rms_xx_0, 0), (self.blocks_divide_xx_0, 0))    
        self.connect((self.blocks_rms_xx_0_0, 0), (self.blocks_divide_xx_0, 1))    
        self.connect((self.blocks_stream_to_vector_0, 0), (self.blocks_null_sink_0, 0))    
        self.connect((self.digital_clock_recovery_mm_xx_0, 0), (self.blocks_complex_to_real_0, 0))    
        self.connect((self.digital_clock_recovery_mm_xx_0, 0), (self.qtgui_const_sink_x_0, 0))    
        self.connect((self.digital_costas_loop_cc_0, 0), (self.digital_clock_recovery_mm_xx_0, 0))    
        self.connect((self.high_pass_filter_0, 0), (self.blocks_rms_xx_0_0, 0))    
        self.connect((self.low_pass_filter_0, 0), (self.analog_agc_xx_0, 0))    
        self.connect((self.osmosdr_source_0, 0), (self.low_pass_filter_0, 0))    
        self.connect((self.root_raised_cosine_filter_0, 0), (self.blocks_rms_xx_0, 0))    
        self.connect((self.root_raised_cosine_filter_0, 0), (self.digital_costas_loop_cc_0, 0))    

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "demod_tcp_qt")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_symbol_rate(self):
        return self.symbol_rate

    def set_symbol_rate(self, symbol_rate):
        self.symbol_rate = symbol_rate
        self.set_sps((self.samp_rate*1.0)/(self.symbol_rate*1.0))
        self.root_raised_cosine_filter_0.set_taps(firdes.root_raised_cosine(1, self.samp_rate, self.symbol_rate, 0.5, 361))
        self.high_pass_filter_0.set_taps(firdes.high_pass(1, self.samp_rate, self.symbol_rate, 300e3, firdes.WIN_BLACKMAN, 6.76))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.set_sps((self.samp_rate*1.0)/(self.symbol_rate*1.0))
        self.root_raised_cosine_filter_0.set_taps(firdes.root_raised_cosine(1, self.samp_rate, self.symbol_rate, 0.5, 361))
        self.qtgui_waterfall_sink_x_0.set_frequency_range(self.center_freq, self.samp_rate)
        self.qtgui_freq_sink_x_0.set_frequency_range(self.center_freq, self.samp_rate)
        self.osmosdr_source_0.set_sample_rate(self.samp_rate*10)
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, self.samp_rate*10, self.samp_rate/2, 100e3, firdes.WIN_HAMMING, 6.76))
        self.high_pass_filter_0.set_taps(firdes.high_pass(1, self.samp_rate, self.symbol_rate, 300e3, firdes.WIN_BLACKMAN, 6.76))

    def get_vgagain(self):
        return self.vgagain

    def set_vgagain(self, vgagain):
        self.vgagain = vgagain
        self.osmosdr_source_0.set_if_gain(self.vgagain, 0)

    def get_sps(self):
        return self.sps

    def set_sps(self, sps):
        self.sps = sps
        self.digital_clock_recovery_mm_xx_0.set_omega(self.sps)

    def get_pll_alpha(self):
        return self.pll_alpha

    def set_pll_alpha(self, pll_alpha):
        self.pll_alpha = pll_alpha
        self.digital_costas_loop_cc_0.set_loop_bandwidth(self.pll_alpha)

    def get_mixgain(self):
        return self.mixgain

    def set_mixgain(self, mixgain):
        self.mixgain = mixgain
        self.osmosdr_source_0.set_bb_gain(self.mixgain, 0)

    def get_lnagain(self):
        return self.lnagain

    def set_lnagain(self, lnagain):
        self.lnagain = lnagain
        self.osmosdr_source_0.set_gain(self.lnagain, 0)

    def get_clock_alpha(self):
        return self.clock_alpha

    def set_clock_alpha(self, clock_alpha):
        self.clock_alpha = clock_alpha
        self.digital_clock_recovery_mm_xx_0.set_gain_omega(self.clock_alpha**2/4.0)
        self.digital_clock_recovery_mm_xx_0.set_gain_mu(self.clock_alpha)

    def get_center_freq(self):
        return self.center_freq

    def set_center_freq(self, center_freq):
        self.center_freq = center_freq
        self.qtgui_waterfall_sink_x_0.set_frequency_range(self.center_freq, self.samp_rate)
        self.qtgui_freq_sink_x_0.set_frequency_range(self.center_freq, self.samp_rate)
        self.osmosdr_source_0.set_center_freq(self.center_freq, 0)


def main(top_block_cls=demod_tcp_qt, options=None):

    from distutils.version import StrictVersion
    if StrictVersion(Qt.qVersion()) >= StrictVersion("4.5.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()
    tb.start()
    tb.show()

    def quitting():
        tb.stop()
        tb.wait()
    qapp.connect(qapp, Qt.SIGNAL("aboutToQuit()"), quitting)
    qapp.exec_()


if __name__ == '__main__':
    main()
