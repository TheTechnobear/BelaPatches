var guiSketch = new p5(function( sketch ) {

    let canvas_dimensions = [sketch.windowWidth, sketch.windowHeight];
    
    sketch.setup = function() {
        sketch.createCanvas(canvas_dimensions[0], canvas_dimensions[1]);
        sketch.background('rgba(28, 232, 181, 0.5)');
        sketch.textSize(sketch.round(sketch.windowWidth/50));
        sketch.textFont('Courier');
        sketch.textAlign(sketch.LEFT, sketch.TOP);

        sketch.text("Calibration Utility",10 ,10);
        sketch.text("A utility for calibration of analog inputs and outputs",10 ,40);

        sketch.text("Board",10 ,70);
        selBoard = sketch.createSelect();
        selBoard.position(100, 70);
        selBoard.option('Salt');
        selBoard.option('Pepper');
        selBoard.option('Bela');
        // selBoard.value('Pepper');
        selBoard.changed( function() {
            Bela.data.sendBuffer(1, 'float', this.elt.selectedIndex);
        });

        sketch.text("Target Voltage", 250, 70);


        targetVolt = inputNum(450,70,100,0.0, function() {
            // body...
        });
 


        // input section
        sketch.text("Input", 10, 100);
        selTargetIn = sketch.createSelect();
        selTargetIn.position(10, 130);
        fillIO(selTargetIn);
        selTargetIn.changed( function() {
            Bela.data.sendBuffer(2, 'float', this.elt.selectedIndex);
        });


        sketch.text("Min", 10, 150);
        sketch.text("Max", 80, 150);
        minInVolt = inputNum(10,170,50,0.0, function() {
            // body...
        });
        maxInVolt = inputNum(80,170,50,0.0, function() {
            // body...
        });

        inDone = sketch.createButton('Calibrated');
        inDone.position(10,200);
        inDone.mousePressed (function() {
            // todo 
        });
 



        // output section
        sketch.text("Output", 450, 100);
        selTargetOut = sketch.createSelect();
        fillIO(selTargetOut);

        // while(selTargetOut.elt.options.length>0) {
        //     selTargetOut.elt.options.remove(0);
        // }

        selTargetOut.option(100);
        selTargetOut.position(450, 130);
        selTargetOut.changed( function() {
            Bela.data.sendBuffer(3, 'float', this.elt.selectedIndex);
        });

        sketch.text("Min", 450, 150);
        sketch.text("Max", 520, 150);
        minOutVolt = inputNum(450,170,50,0.0, function() {
            statusLine(this.value);
        });

        maxOutVolt = inputNum(520,170,50,0.0, function() {
            // body...
        });

        outDone = sketch.createButton('Calibrated');
        outDone.position(450,200);
        outDone.mousePressed (function() {
            // todo 
        });

  //       sketch.textAlign(sketch.LEFT, sketch.TOP);

  //       string = "offset : "
  //       sketch.text(string,0, 150);
        
  //       sliderOffset = sketch.createSlider(-1, 1, 0,0.0001);
  //       sliderOffset.position(0, 200);
  //       sliderOffset.size(300);

    }

    function statusLine(v) {
        sketch.rect(0,600,sketch.textWidth('AAAAAAAAAAAAAAAAAAA') , sketch.textSize() * 1.1);
        sketch.text(v, 0,600);
    }

    sketch.draw = function() {
     
        // sketch.textAlign(sketch.LEFT, sketch.TOP);
        // let offset = Bela.data.buffers[0];
        // string = offset;
        // sketch.rect(190,150,sketch.textWidth('AAAAAAAAAAAAAAAAAAA') , sketch.textSize() * 1.1);
        // sketch.text(string, 200,150);
        // sliderOffset.value(offset);
        
        // sketch.noLoop();
    }

    function inputNum(x,y,sz,init,fn) {
        let c= sketch.createInput(init,"number");
        c.position(x,y);
        c.size(sz);
        // c.input(fn);
        c.elt.addEventListener("focusout", fn);
        return c;
    }
    
    function fillIO(sel) {
        sel.option(1);
        sel.option(2);
        sel.option(3);
        sel.option(4);
        sel.option(5);
        sel.option(6);
        sel.option(7);
        sel.option(8);
        sel.value(1);
    }
    
    function selectTargetIn() {

    }

    // function selectTargetOut() {

    // }

 //    function selectBoardEvent() {
 //        sketch.textAlign(sketch.LEFT, sketch.TOP);
	// 	let item = selBoard.value();
	// 	// sketch.rect(190,300, sketch.textWidth('AAAAAAAA') , sketch.textSize() * 1.1);
	// 	// sketch.text(item, 200,300);
	// 	let idx = 0;
	// 	if (item=='Salt') idx = 0;
	// 	else if (item=='Pepper') idx = 1;
	// 	else if (item=='Bela') idx = 2;
	// 	else idx=4;
		
 //    	Bela.data.sendBuffer(1, 'float', idx);	
	// }
}, 'gui');
	
