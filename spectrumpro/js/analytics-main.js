//-------------------------------------------------
//      Redpitaya analytics system
//      Created by Alexey Kaygorodov
//-------------------------------------------------

(function($) {
	var startUsing = 0;
    $(document).ready(function($) {
        AnalyticsCore.init(function(){
            AnalyticsCore.sendExecTime('/spectrumpro', 'spectrumpro');
            AnalyticsCore.sendScreenView('/spectrumpro', 'Redpitaya', 'Spectrum analyser Pro');
            AnalyticsCore.sendSysInfo('/spectrumpro');
            startUsing = performance.now();
        });
    });

	$(window).on('beforeunload', function(){
	      $.cookie('spectrumpro-run', performance.now() - startUsing);
	});

})(jQuery);
