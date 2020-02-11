var guiSketch = new p5(function( sketch ) {

    let string = "";
    let canvas_dimensions = [sketch.windowWidth, sketch.windowHeight];
    
    let sliderOffset;
    let selBoard;
    
    sketch.setup = function() {
        sketch.createCanvas(canvas_dimensions[0], canvas_dimensions[1]);
        sketch.background('rgba(28, 232, 181, 0.5)');
        sketch.textSize(sketch.round(sketch.windowWidth/50));
        sketch.textFont('Courier');
        sketch.textAlign(sketch.CENTER, sketch.CENTER);

		string = "Calibration Utility\n"
		string = string + "A utility to help calibration analog inputs and outputs";
        sketch.text(string,sketch.width/2 ,50);

        sketch.textAlign(sketch.LEFT, sketch.TOP);
        string = "offset : "
        sketch.text(string,0, 150);
        
        sliderOffset = sketch.createSlider(-1, 1, 0,0.0001);
        sliderOffset.position(0, 200);
        sliderOffset.size(300);
        
        
		selBoard = sketch.createSelect();
		selBoard.position(0, 300);
		selBoard.option('Salt');
		selBoard.option('Pepper');
		selBoard.option('Bela');
		selBoard.changed(selectBoardEvent);
		selBoard.value('Pepper');
		// selectBoardEvent();

    };

    sketch.draw = function() {
     
        sketch.textAlign(sketch.LEFT, sketch.TOP);
        let offset = Bela.data.buffers[0];
        string = offset;
        sketch.rect(190,150,sketch.textWidth('AAAAAAAAAAAAAAAAAAA') , sketch.textSize() * 1.1);
        sketch.text(string, 200,150);
        sliderOffset.value(offset);
        
        // sketch.noLoop();
    };
    
    
    function selectBoardEvent() {
        sketch.textAlign(sketch.LEFT, sketch.TOP);
		let item = selBoard.value();
		sketch.rect(190,300, sketch.textWidth('AAAAAAAA') , sketch.textSize() * 1.1);
		sketch.text(item, 200,300);
		let idx = 0;
		if (item=='Salt') idx = 0;
		else if (item=='Pepper') idx = 1;
		else if (item=='Bela') idx = 2;
		else idx=4;
		
    	Bela.data.sendBuffer(1, 'float', idx);	
	}
}, 'gui');
	