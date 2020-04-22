var guiSketch = new p5(function( p ) {

    const dataIn = {
        DI_BOARD : 0,
        DI_I_TARGET_CH : 1,
        DI_I_TARGET_VOLT : 2,
        DI_I_CAL_TRIG : 3,
        DI_O_TARGET_CH : 4,
        DI_O_TARGET_VOLT : 5, 
        DI_O_MIN_VOLT : 6,
        DI_O_MAX_VOLT : 7,
        DI_O_T_FLOAT : 8,
        DI_O_CAL_TRIG : 9,
        DI_MAX : 10
    };


    const dataOut = {
        DO_I_MIN_VOLT : 0 ,
        DO_I_MAX_VOLT : 1 ,
        DO_I_THEORY_VOLT : 2,
        DO_I_ACT_FLOAT : 3 ,
        DO_I_CAL_FLOAT : 4,
        DO_I_CAL_VOLT : 5,
        DO_O_ACT_FLOAT : 6,
        DO_O_CAL_FLOAT : 7,
        DO_O_MIN_VOLT : 8,
        DO_O_MAX_VOLT : 9,
        DO_MAX : 10
    };

    const boardType = {
        BT_SALT : 0,
        BT_PEPPER : 1 ,
        BT_BELA : 2 ,
        BT_MAX : 3
    };


    var canvas_dimensions = [p.windowWidth, p.windowHeight];
    var fH = 0;
    var fW = 0;
    var dataBuffer=new Float32Array(dataIn.DI_MAX);


    Bela.data.target.addEventListener('buffer-ready', function(event) {
        if(event.detail ==0 && typeof Bela.data.buffers[0] != 'undefined') {
            redraw();
        }
    });

    var voltTextPos=[0,0];
    var inTheoryVolt=[0,0];
    var inActFloat=[0,0];
    var inCalFloat=[0,0];
    var inCalVolt=[0,0];
    var outActFloat=[0,0];
    var outCalFloat=[0,0];

    p.setup = function() {
        p.createCanvas(canvas_dimensions[0], canvas_dimensions[1]);
        p.background(160);
        p.fill(0);
        p.textSize(14);
        p.textFont('Courier');
        p.textAlign(p.LEFT, p.TOP);

        fW=p.textWidth('A') * 1.1;
        fH=p.textSize() * 1.5;

        tR = fH;
        tC = fW;
        p.text("Calibration Utility",tC,tR); 
        tR += fH;

        p.text("A utility for calibration of analog inputs and outputs",tC,tR);
        tR += fH;

        p.text("Board",tC ,tR); tC+= fW*5;
        selBoard = p.createSelect();
        selBoard.position(tC, tR);
        selBoard.option('Salt');
        selBoard.option('Pepper');
        selBoard.option('Bela');
        // selBoard.value('Pepper');
        selBoard.changed( function() {
            dataBuffer[dataIn.DI_BOARD] = this.elt.selectedIndex;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });
        dataBuffer[dataIn.DI_BOARD] = 0;

        tR+=fH;
        tC=fW;
        p.text("Voltage ", tC, tR)
        tC+=fW* 10;
        voltTextPos = [tC,tR]
        p.text("In: Na/Na Out: Na/Na",voltTextPos[0],voltTextPos[1]);

        tC= fW;
        tR+= fH*3;

        contentC=tC;
        contentR=tR;

        drawInput(contentC,contentR);
        drawOutput(contentC + fW*40,contentR);

        Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
    }

    function statusLine(v) {
        p.rect(0,600,p.textWidth('AAAAAAAAAAAAAAAAAAA') , p.textSize() * 1.1);
        p.text(v, 0,600);
    }

    function drawNum(x,y,str) {
        p.stroke(160);
        p.fill(160);
        p.rect(x-fW,y,fW*20,fW*20,fH);
        p.fill(0);
        p.text(str,x,y);
    }

    function redraw() {
        var buffer = Bela.data.buffers[0];
        if(buffer.length==dataOut.DO_MAX) {
            p.stroke(160);
            p.fill(160);
            tC=voltTextPos[0];tR=voltTextPos[1];
            p.rect(tC,tR,fW*20,fH);
            minIV=buffer[dataOut.DO_I_MIN_VOLT];
            maxIV=buffer[dataOut.DO_I_MAX_VOLT];
            minOV=buffer[dataOut.DO_O_MIN_VOLT];
            maxOV=buffer[dataOut.DO_O_MAX_VOLT];
            str = "In: "  + minIV.toString() + "/" + maxIV.toString()+ "";
            str += "    ";
            str +="Out: " + minOV.toString() + "/" + maxOV.toString()+ "";
            p.fill(0);
            p.text(str,tC,tR);

            tC=inTheoryVolt[0];tR=inTheoryVolt[1];
            drawNum(tC,tR,buffer[dataOut.DO_I_THEORY_VOLT]);

            tC=inActFloat[0];tR=inActFloat[1];
            drawNum(tC,tR,buffer[dataOut.DO_I_ACT_FLOAT]);
            tC=inCalFloat[0];tR=inCalFloat[1];
            drawNum(tC,tR,buffer[dataOut.DO_I_CAL_FLOAT]);
            tC=inCalVolt[0];tR=inCalVolt[1];
            drawNum(tC,tR,buffer[dataOut.DO_I_CAL_VOLT]);
            tC=outActFloat[0];tR=outActFloat[1];
            drawNum(tC,tR,buffer[dataOut.DO_O_ACT_FLOAT]);
            tC=outCalFloat[0];tR=outCalFloat[1];
            drawNum(tC,tR,buffer[dataOut.DO_O_CAL_FLOAT]);
        }
    }

    p.draw = function() {
        // p.noLoop();
    }


    function drawInput(inC,inR) {
        tC=inC;
        tR=inR;

        p.text("Input", tC, tR);
        tC+= fW*15;
        selTargetIn = p.createSelect();
        selTargetIn.position(tC, tR);
        fillIO(selTargetIn);
        selTargetIn.changed( function() {
            console.log(this.elt.value);
            dataBuffer[dataIn.DI_I_TARGET_CH] = this.elt.value;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });

        tC=inC;
        tR+=fH;

        p.text("Target (v)", tC,tR);
        tC+= fW*15;

        targetInVolt = inputNum(tC,tR,100,0.0, function() {
            dataBuffer[dataIn.DI_I_TARGET_VOLT] = this.value;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });

        tR+= fH;
        tC=inC;

        p.text("Theory (v)", tC,tR);
        tC+= fW*15;

        inTheoryVolt=[tC,tR];
        p.text("0.001", tC,tR);
        tR+= fH;

        tR+=2*fH;

        inDone = p.createButton('Calibrated');
        inDone.position(tC,tR);
        inDone.mouseReleased(function() {
            dataBuffer[dataIn.DI_I_CAL_TRIG] = 0;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });
        inDone.mousePressed (function() {
            dataBuffer[dataIn.DI_I_CAL_TRIG] = 1;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });
 
        tR+= fH * 3;
        tC=inC;
        p.text("actual (dec)", tC,tR); 
        tC+= fW*15;
        inActFloat=[tC,tR];;
        p.text("0.001", tC,tR);
        tR+= fH;

        tC=inC;
        p.text("calibrated (dec)", tC,tR); 
        tC+= fW*15;
        inCalFloat=[tC,tR];;
        p.text("0.001", tC,tR);
        tR+= fH;

        tC=inC;
        p.text("calibrated (v)", tC,tR); 
        tC+= fW*15;
        inCalVolt=[tC,tR];;
        p.text("0.001", tC,tR);
        tR+= fH;

    }

    function drawOutput(inC,inR) {
        tC=inC;
        tR=inR;

        var outFloat=0.5;

        p.text("Output", tC, tR);
        tC+= fW*15;
        selTargetOut = p.createSelect();
        selTargetOut.position(tC, tR);
        fillIO(selTargetOut);
        selTargetOut.changed( function() {
            dataBuffer[dataIn.DI_O_TARGET_CH] = this.elt.value
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });

        tC=inC;
        tR+=fH;

        p.text("Target (v)", tC,tR);
        tC+= fW*15;

        targetOutVolt = inputNum(tC,tR,100,0.0, function() {
            dataBuffer[dataIn.DI_O_TARGET_VOLT] = this.value;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });

        tR+= fH;
        p.text("Min", tC, tR);
        minOutVolt = inputNum(tC,tR+fH,fW*5,0.0, function() {
            dataBuffer[dataIn.DI_O_MIN_VOLT] = this.value;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });
        p.text("Max", tC + fW*10, tR);
        maxOutVolt = inputNum(tC + fW*10,tR+fH,fW*5,0.0, function() {
            dataBuffer[dataIn.DI_O_MAX_VOLT] = this.value;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });


        tC=inC;
        tR+=fH*3;

        outDownCourse = p.createButton('<<');
        outDownCourse.position(tC,tR);
        outDownCourse.mousePressed (function() {
            val = dataBuffer[dataIn.DI_O_T_FLOAT];
            val -= 0.05;
            if(val<0) val=0.0;
            dataBuffer[dataIn.DI_O_T_FLOAT] = val;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });
        tC+= 5*fW;
        outDownFine = p.createButton('<');
        outDownFine.position(tC,tR);
        outDownFine.mousePressed (function() {
            val = dataBuffer[dataIn.DI_O_T_FLOAT];
            val -= 0.005;
            if(val<0) val=0.0;
            dataBuffer[dataIn.DI_O_T_FLOAT] = val;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });
        tC+= 5*fW;

        outDone = p.createButton('Calibrated');
        outDone.position(tC,tR);
        outDone.mouseReleased(function() {
            dataBuffer[dataIn.DI_O_CAL_TRIG] = 0;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });
        outDone.mousePressed (function() {
            dataBuffer[dataIn.DI_O_CAL_TRIG] = 1;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });

        tC+= 10*fW;
        outUpFine = p.createButton('>');
        outUpFine.position(tC,tR);
        outUpFine.mousePressed (function() {
            val = dataBuffer[dataIn.DI_O_T_FLOAT];
            val += 0.005;
            if(val>1.0) val=1.0;
            dataBuffer[dataIn.DI_O_T_FLOAT] = val;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });
        tC+= 5*fW;
        outUpCourse = p.createButton('>>');
        outUpCourse.position(tC,tR);
        outUpCourse.mousePressed (function() {
            val = dataBuffer[dataIn.DI_O_T_FLOAT];
            val += 0.01;
            if(val>1.0) val=1.0;
            dataBuffer[dataIn.DI_O_T_FLOAT] = val;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
       });


        tR+= fH * 3;
        tC=inC;
        p.text("actual (dec)", tC,tR); 
        tC+= fW*15;
        outActFloat=[tC,tR];;
        p.text("0.001", tC,tR);
        tR+= fH;

        tC=inC;
        p.text("calibrated (dec)", tC,tR); 
        tC+= fW*15;
        outCalFloat=[tC,tR];;
        p.text("0.001", tC,tR);
        tR+= fH;

    }

    function inputNum(x,y,sz,init,fn) {
        let c= p.createInput(init,"number");
        c.position(x,y);
        c.size(sz);
        // c.input(fn);
        c.elt.addEventListener("focusout", fn);
        return c;
    }
    
    function fillIO(sel) {
        sel.option(0);
        sel.option(1);
        sel.option(2);
        sel.option(3);
        sel.option(4);
        sel.option(5);
        sel.option(6);
        sel.option(7);
        sel.value(0);
    }

}, 'gui');
	
