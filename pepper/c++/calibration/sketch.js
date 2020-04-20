var guiSketch = new p5(function( sketch ) {

    let canvas_dimensions = [sketch.windowWidth, sketch.windowHeight];
    let fH = 0;
    let fW = 0;
    
    sketch.setup = function() {
        sketch.createCanvas(canvas_dimensions[0], canvas_dimensions[1]);
        sketch.background('rgba(28, 232, 181, 0.5)');
        sketch.textSize(14);
        sketch.textFont('Courier');
        sketch.textAlign(sketch.LEFT, sketch.TOP);

        fW=sketch.textWidth('A') * 1.1;
        fH=sketch.textSize() * 1.5;

        tR = fH;
        tC = fW;
        sketch.text("Calibration Utility",tC,tR); 
        tR += fH;

        sketch.text("A utility for calibration of analog inputs and outputs",tC,tR);
        tR += fH;

        sketch.text("Board",tC ,tR); tC+= fW*5;
        selBoard = sketch.createSelect();
        selBoard.position(tC, tR);
        selBoard.option('Salt');
        selBoard.option('Pepper');
        selBoard.option('Bela');
        // selBoard.value('Pepper');
        selBoard.changed( function() {
            Bela.data.sendBuffer(1, 'float', this.elt.selectedIndex);
        });



        tR+= fH*3;
        tC= fW;

        contentC=tC;
        contentR=tR;

        drawInput(contentC,contentR);
        drawOutput(contentC + fW*40,contentR);


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


    function drawInput(inC,inR) {
        tC=inC;
        tR=inR;

        sketch.text("Input", tC, tR);
        tC+= fW*15;
        selTargetIn = sketch.createSelect();
        selTargetIn.position(tC, tR);
        fillIO(selTargetIn);
        selTargetIn.changed( function() {
            Bela.data.sendBuffer(2, 'float', this.elt.selectedIndex);
        });

        tC=inC;
        tR+=fH;

        sketch.text("Target (v)", tC,tR);
        tC+= fW*15;

        targetInVolt = inputNum(tC,tR,100,0.0, function() {
            // body...
        });

        tR+= fH;
        tC=inC;

        sketch.text("Theory (v)", tC,tR);
        tC+= fW*15;
        sketch.text("0.001", tC,tR);
        tR+= fH;

        tR+=2*fH;

        inDone = sketch.createButton('Calibrated');
        inDone.position(tC,tR);
        inDone.mousePressed (function() {
            // todo 
        });
 
        tR+= fH * 3;
        tC=inC;
        sketch.text("actual (dec)", tC,tR); 
        tC+= fW*15;
        sketch.text("0.001", tC,tR);
        tR+= fH;

        tC=inC;
        sketch.text("calibrated (dec)", tC,tR); 
        tC+= fW*15;
        sketch.text("0.001", tC,tR);
        tR+= fH;

        tC=inC;
        sketch.text("calibrated (v)", tC,tR); 
        tC+= fW*15;
        sketch.text("0.001", tC,tR);
        tR+= fH;

    }

    function drawOutput(inC,inR) {
        tC=inC;
        tR=inR;

        sketch.text("Output", tC, tR);
        tC+= fW*15;
        selTargetOut = sketch.createSelect();
        selTargetOut.position(tC, tR);
        fillIO(selTargetOut);
        selTargetOut.changed( function() {
            Bela.data.sendBuffer(3, 'float', this.elt.selectedIndex);
        });

        tC=inC;
        tR+=fH;

        sketch.text("Target (v)", tC,tR);
        tC+= fW*15;

        targetOutVolt = inputNum(tC,tR,100,0.0, function() {
            // body...
        });

        tR+= fH;
        sketch.text("Min", tC, tR);
        minOutVolt = inputNum(tC,tR+fH,fW*5,0.0, function() {
            // body...
        });
        sketch.text("Max", tC + fW*10, tR);
        maxOutVolt = inputNum(tC + fW*10,tR+fH,fW*5,0.0, function() {
            // body...
        });


        tC=inC;
        tR+=fH*3;

        outDownCourse = sketch.createButton('<<');
        outDownCourse.position(tC,tR);
        outDownCourse.mousePressed (function() {
            // todo 
        });
        tC+= 5*fW;
        outDownFine = sketch.createButton('<');
        outDownFine.position(tC,tR);
        outDownFine.mousePressed (function() {
            // todo 
        });
        tC+= 5*fW;
        outDone = sketch.createButton('Calibrated');
        outDone.position(tC,tR);
        outDone.mousePressed (function() {
            // todo 
        });
        tC+= 10*fW;
        outUpFine = sketch.createButton('>');
        outUpFine.position(tC,tR);
        outUpFine.mousePressed (function() {
            // todo 
        });
        tC+= 5*fW;
        outUpCourse = sketch.createButton('>>');
        outUpCourse.position(tC,tR);
        outUpCourse.mousePressed (function() {
            // todo 
        });


        tR+= fH * 3;
        tC=inC;
        sketch.text("actual (dec)", tC,tR); 
        tC+= fW*15;
        sketch.text("0.001", tC,tR);
        tR+= fH;

        tC=inC;
        sketch.text("calibrated (dec)", tC,tR); 
        tC+= fW*15;
        sketch.text("0.001", tC,tR);
        tR+= fH;

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
        sel.option('Min');
        sel.option(1);
        sel.option(2);
        sel.option(3);
        sel.option(4);
        sel.option(5);
        sel.option(6);
        sel.option(7);
        sel.option(8);
        sel.option('Max');
        sel.value('Min');
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
	
