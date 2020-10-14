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
        DO_I_ACT_VOLT : 2,
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
        if(event.detail ==1 && typeof Bela.data.buffers[1] != 'undefined') {
            redrawStatus();
        }
    });

    var voltTextPos=[0,0];
    var inActVolt=[0,0];
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
        p.noLoop();
    }
    
    function redrawStatus() {
        var buffer = Bela.data.buffers[1];
        statusLine(buffer);
    }

    function statusLine(v) {
        var str = v.join("");
        var x=0;
        var y=600;
        var ch=120

        p.stroke(160);
        p.fill(160);
        p.rect(x-fW,y,fW*ch,fW*ch,fH);
        p.fill(0);
        p.text(str,x,y);

    }

    function drawNum(x,y,str) {
        p.stroke(160);
        p.fill(160);
        p.rect(x-fW,y,fW*20,fW*20,fH);
        p.fill(0);
        p.text(str,x,y);
    }

    function fillVolt(ctrl, minv, maxv, step) {
      var i=ctrl.elt.selectedIndex;
      while(ctrl.elt.options.length>0) {
        ctrl.elt.remove(0);
      }
      s = (p.abs(minv) + maxv ) / step;
      for(v = minv; v <=maxv; v = v+s ) {
        ctrl.option(v);
      }
      ctrl.elt.selectedIndex=i;
    }

    var minIV=-10;
    var maxIV=-10;
    var minOV=-10;
    var maxOV=-10;

    function redraw() {
        var buffer = Bela.data.buffers[0];
        if(buffer.length==dataOut.DO_MAX) {
            p.stroke(160);
            p.fill(160);
            tC=voltTextPos[0];tR=voltTextPos[1];
            p.rect(tC,tR,fW*20,fH);

            var chg=false;
            chg = chg || minIV!=buffer[dataOut.DO_I_MIN_VOLT];
            minIV=buffer[dataOut.DO_I_MIN_VOLT];
            chg = chg || maxIV!=buffer[dataOut.DO_I_MAX_VOLT];
            maxIV=buffer[dataOut.DO_I_MAX_VOLT];
            if(chg) {
                fillVolt(targetInVolt,minIV,maxIV,10.0);
                dataBuffer[dataIn.DI_I_TARGET_VOLT] = targetInVolt.elt.value;
                Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
            }

            chg=false;
            chg = chg || minOV!=buffer[dataOut.DO_O_MIN_VOLT];
            minOV=buffer[dataOut.DO_O_MIN_VOLT];
            chg = chg || maxOV!=buffer[dataOut.DO_O_MAX_VOLT];
            maxOV=buffer[dataOut.DO_O_MAX_VOLT];
            if(chg) {
                fillVolt(targetOutVolt,minOV,maxOV,10.0);
                dataBuffer[dataIn.DI_O_TARGET_VOLT] = targetOutVolt.elt.value;
                Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
            }
            str = "In: "  + minIV.toString() + "/" + maxIV.toString()+ "";
            str += "    ";
            str +="Out: " + minOV.toString() + "/" + maxOV.toString()+ "";
            p.fill(0);
            p.text(str,tC,tR);

            tC=inActVolt[0];tR=inActVolt[1];
            drawNum(tC,tR,buffer[dataOut.DO_I_ACT_VOLT]);
            tC=inActFloat[0];tR=inActFloat[1];
            drawNum(tC,tR,buffer[dataOut.DO_I_ACT_FLOAT]);

            tC=inCalVolt[0];tR=inCalVolt[1];
            drawNum(tC,tR,buffer[dataOut.DO_I_CAL_VOLT]);
            tC=inCalFloat[0];tR=inCalFloat[1];
            drawNum(tC,tR,buffer[dataOut.DO_I_CAL_FLOAT]);

            tC=outActFloat[0];tR=outActFloat[1];
            drawNum(tC,tR,buffer[dataOut.DO_O_ACT_FLOAT]);
            tC=outCalFloat[0];tR=outCalFloat[1];
            drawNum(tC,tR,buffer[dataOut.DO_O_CAL_FLOAT]);

            dataBuffer[dataIn.DI_O_T_FLOAT] = buffer[dataOut.DO_O_CAL_FLOAT];
        }
    }

    p.draw = function() {
        p.noLoop();
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
            dataBuffer[dataIn.DI_I_TARGET_CH] = this.elt.value;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });

        tC=inC;
        tR+=fH;

        p.text("Target (v)", tC,tR);
        tC+= fW*15;

        targetInVolt = p.createSelect();
        targetInVolt.position(tC, tR);
        targetInVolt.changed( function() {
            dataBuffer[dataIn.DI_I_TARGET_VOLT] = this.elt.value;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
        });

        tC=inC;
        tR+=4*fH;

        inDone = p.createButton('Calibrated');
        inDone.position(tC,tR);
        inDone.mousePressed (function() {
            dataBuffer[dataIn.DI_I_CAL_TRIG] = 1;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
            setTimeout(function() {
                dataBuffer[dataIn.DI_I_CAL_TRIG] = 0;
                Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
            },500);
        });
 
        tR+= fH * 3;

        tC=inC;
        p.text("actual (v)", tC,tR);
        tC+= fW*15;
        inActVolt=[tC,tR];
        p.text("0.001", tC,tR);
        tR+= fH;

        tC=inC;
        p.text("actual (dec)", tC,tR); 
        tC+= fW*15;
        inActFloat=[tC,tR];;
        p.text("0.001", tC,tR);
        tR+= fH;

        tC=inC;
        p.text("calibrated (v)", tC,tR); 
        tC+= fW*15;
        inCalVolt=[tC,tR];;
        p.text("0.001", tC,tR);
        tR+= fH;

        tC=inC;
        p.text("calibrated (dec)", tC,tR); 
        tC+= fW*15;
        inCalFloat=[tC,tR];;
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

        targetOutVolt = p.createSelect();
        targetOutVolt.position(tC, tR);
        targetOutVolt.changed( function() {
            // set the target value too
            // var val = this.elt.selectedIndex / 10.0;
            // dataBuffer[dataIn.DI_O_T_FLOAT] = val;
            dataBuffer[dataIn.DI_O_TARGET_VOLT] = this.elt.value;
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
        outDone.mousePressed (function() {
            dataBuffer[dataIn.DI_O_CAL_TRIG] = 1;
            Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
            setTimeout(function() {
                dataBuffer[dataIn.DI_O_CAL_TRIG] = 0;
                Bela.data.sendBuffer(0,'float',Array.from(dataBuffer));
            },500);
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
	
