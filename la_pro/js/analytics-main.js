//-------------------------------------------------
//      Redpitaya analytics system
//      Created by Alexey Kaygorodov
//-------------------------------------------------

(function($) {
	var startUsing = 0;
    $(document).ready(function($) {
        AnalyticsCore.init(function(){
            AnalyticsCore.sendExecTime('/la_pro', 'la_pro');
            AnalyticsCore.sendScreenView('/la_pro', 'Redpitaya', 'Logic analyser');
            AnalyticsCore.sendSysInfo('/la_pro');
            startUsing = performance.now();
        });
    });

	$(window).on('beforeunload', function(){
	      $.cookie('la_propro-run', performance.now() - startUsing);
	});

})(jQuery);
