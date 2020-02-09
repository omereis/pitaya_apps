/* 

Javascript waterfall library, version 0.1.

*/




// Waterfall code
(function($) {

    function Waterfall(placeholder, width, height) {
        
        // Waterfall object
        var waterfall = this;




        // Variables
        var newCanvas = $('<canvas/>',{'class':'waterfall_canvas'});
        placeholder.append(newCanvas);

        waterfall.canvas = newCanvas[0];
        waterfall.canvas_context = waterfall.canvas.getContext('2d');
        waterfall.canvas_context.canvas.width = width;
        waterfall.canvas_context.canvas.height = height;
        waterfall.canvas_context.imageSmoothingEnabled= true;
        waterfall.canvas_context.filter = "blur(3px)";
        waterfall.width = width;
        waterfall.height = height;
        waterfall.data = [];
        waterfall.canvasData = waterfall.canvas_context.getImageData(0, 0, width, height);




        // Functions 
        function hue2rgb(p, q, t){
            if(t < 0) t += 1;
            if(t > 1) t -= 1;
            if(t < 1/6) return p + (q - p) * 6 * t;
            if(t < 1/2) return q;
            if(t < 2/3) return p + (q - p) * (2/3 - t) * 6;
            return p;
        }


        waterfall.drawPixel = function(x, y, r, g, b, a) {
            var index = (x + y * this.width) * 4;

            this.canvasData.data[index + 0] = r;
            this.canvasData.data[index + 1] = g;
            this.canvasData.data[index + 2] = b;
            this.canvasData.data[index + 3] = a;
        }


        waterfall.updateCanvas = function() {
            this.canvas_context.putImageData(this.canvasData, 0, 0);
        }


        waterfall.cleanData = function() {

            for (y = 0; y < this.height; y++){
                 for (x = 0; x < this.width; x++){

                    var index = (x + y * this.width) * 4;
                    this.canvasData.data[index + 0] = 0;
                    this.canvasData.data[index + 1] = 0;
                    this.canvasData.data[index + 2] = 255;
                    this.canvasData.data[index + 3] = 255;
                }
            }
        }


        waterfall.draw = function() { 
            if (!waterfall.data) return; 
            var data_size = 0;
            var koef = 1;
            var max = 999;
            var min = -999;
            var range = 0;
            var value = 0;
            var percent = 0;

            // Clean previous data
            this.cleanData();

            // Draw new data
            for (y = this.data.length - 1; y > 0; y--){

                data_size = this.data[y].length;
                koef = this.width / data_size;

                max = Math.max.apply(null, waterfall.data[y]);
                min = Math.min.apply(null, waterfall.data[y]);
                range = max - min;

                for (x = 0; x < this.width; x++){

                    value = waterfall.data[y][Math.floor(x / koef)];
                    percent = (value - min) / range;

                    var h = 0.7 - percent * 0.7;
                    var s = 1.0;
                    var l = 0.5;

                    var q = l < 0.5 ? l * (1 + s) : l + s - l * s;
                    var p = 2 * l - q;

                    r = Math.round(hue2rgb(p, q, h + 1/3) * 255);
                    g = Math.round(hue2rgb(p, q, h)       * 255);
                    b = Math.round(hue2rgb(p, q, h - 1/3) * 255);


                    this.drawPixel(x, (this.data.length -1) - y, r, g, b, 255);       
                }
            }
            this.updateCanvas();
        };


        waterfall.addData = function(data) { 
            this.data.push(data);
            if (this.data.length > this.height) this.data.splice(0, 1);
        };


        waterfall.setSize = function(width, height) { 
            
            this.canvas_context.canvas.width = width;
            this.canvas_context.canvas.height = height;
            this.width = width;
            this.height = height;
            this.canvasData = this.canvas_context.getImageData(0, 0, width, height);
        };
    }



    $.createWaterfall = function(placeholder, width, height) {
        var waterfall = new Waterfall($(placeholder), width, height);
        return waterfall;
    };

})(jQuery);