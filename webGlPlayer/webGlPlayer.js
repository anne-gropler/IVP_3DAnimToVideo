var gl;

var verticesBuffer;
var verticesTextureCoordBuffer;
var verticesIndexBuffer;

var mvMatrix;
var shaderProgram;
var vertexPositionAttribute;
var textureCoordAttribute;
var perspectiveMatrix;

var numberOfVideosLoaded;
var framesPerSecond;
var colorTexture;
var normalsTexture;
var depthTexture;

var videoElement;
var videoNormalsElement;
var videoDepthElement;


/**
 * Called when the canvas is created
 */
function start()
{
  framesPerSecond = 0;
  numberOfVideosLoaded = 0;
    
  var canvas = document.getElementById("glcanvas");
  
  //Slider to update the x-position of the light
  var slideX = document.getElementById('slideX');  
  slideX.onchange = function()
  {
        var lightPositionX = this.value;
        gl.uniform1f(gl.getUniformLocation(shaderProgram, "uLightPositionX"), lightPositionX);
  };
  
  //Get the HTML5 video element
  videoElement = document.getElementById("video");
  videoNormalsElement = document.getElementById("videoNormals");
  videoDepthElement = document.getElementById("videoDepth");

  //Initialize the GL context and set the variable gl
  initWebGL(canvas);
  
  if (gl)
  {
    //WebGL is available
    gl.clearColor(0.0, 0.0, 0.0, 1.0);  //Clear to black, fully opaque
    gl.clearDepth(1.0);                 //Clear everything
    gl.enable(gl.DEPTH_TEST);           //Enable depth testing
    gl.depthFunc(gl.LEQUAL);            //Near things obscure far things
    
    initShaders();

    initBuffers();
    
    initTextures();

    //Start listening for the canplaythrough-event, so we don't
    //start playing the video until we can do so without stuttering
    videoElement.addEventListener("canplaythrough", startVideo, true);
    videoNormalsElement.addEventListener("canplaythrough", startVideo, true);
    videoDepthElement.addEventListener("canplaythrough", startVideo, true);
    
    //Start listening for the ended-event, so we can stop the
    //animation when the video is finished playing.
    //It is just pro forma because the video should loop
    //Chrome doesn't loop, but doesn't fire the ended-event neither
    videoElement.addEventListener("ended", videoDone, true);
    videoNormalsElement.addEventListener("ended", videoDone, true);
    videoDepthElement.addEventListener("ended", videoDone, true);

    //Video sources are loaded when the page loads
    //CrossOrigin anonymous so that we can open index.html without netbeans
    video.preload = "auto";
    videoElement.src = "presentation_color_small.ogv";
    videoElement.crossOrigin = "anonymous";
    videoNormals.preload = "auto";
    videoNormalsElement.src = "presentation_normals_small.ogv";
    videoNormalsElement.crossOrigin = "anonymous";
    videoDepth.preload = "auto";
    videoDepthElement.src = "presentation_depth_small.ogv";
    videoDepthElement.crossOrigin = "anonymous";
    
    //Which layer of the video is shown
    var layer = 0;
    
    window.onkeyup = function(e)
    {
        var key = e.keyCode ? e.keyCode : e.which;
        
        switch (key)
        {
            case 81: //q
                layer = 0; //color-layer
                break;
            case 87: //w
                layer = 1; //normals-layer
                break;
            case 69: //e
                layer = 2; //depth-layer
                break;
            case 82: //r
                layer = 3; //post-processing (shading)
                break;
        }
        gl.uniform1i(gl.getUniformLocation(shaderProgram, "uLayer"), layer);
    };
    
    //Debugging: print framerate console every second
    setInterval(function()
    {
        console.log("Framerate: " + framesPerSecond);
        framesPerSecond=0;
    }, 1000);
  }
}

/** Initialize WebGL, setting gl as the GL context
 * or null if WebGL isn't available or could not be initialized.
 * @param {HTMLCanvasElement} canvas
 */
function initWebGL(canvas)
{
  gl = null;
  
  try
  {
    gl = canvas.getContext("experimental-webgl") || canvas.getContext("webgl");
  }
  catch(e)
  {
  }

  if (!gl)
  {
    alert("Unable to initialize WebGL. This Webbrowser possibly does not support it.\nHint: See http://caniuse.com/webgl");
  }
}

/**
 * Initialize Buffers and fill them to draw a quad
 */
function initBuffers()
{
  verticesBuffer = gl.createBuffer();
  
  // Select the verticesBuffer as the one to apply vertex
  // operations to from here out.
  gl.bindBuffer(gl.ARRAY_BUFFER, verticesBuffer);
   
  var vertices = [
    -3.0, -3.0,  -1.5,
     3.0, -3.0,  -1.5,    
     3.0,  3.0,  -1.5,
    -3.0,  3.0,  -1.5
  ];
  
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);

  // Map the video texture onto the face.  
  verticesTextureCoordBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, verticesTextureCoordBuffer);
  
  var textureCoordinates = [
    0.0,  0.0,
    1.0,  0.0,
    1.0,  1.0,
    0.0,  1.0
  ];

  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(textureCoordinates), gl.STATIC_DRAW);

  verticesIndexBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, verticesIndexBuffer);
  
  var vertexIndices = [
    0,  1,  2,
    0,  2,  3
  ];
  
  gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(vertexIndices), gl.STATIC_DRAW);
}

/**
 * Initialize the video textures
 */
function initTextures()
{
  colorTexture = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, colorTexture);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
  
  normalsTexture = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, normalsTexture);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
  
  depthTexture = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, depthTexture);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
  
  gl.bindTexture(gl.TEXTURE_2D, null);
}

/**
 * Update video texture, so that the used texture contains the latest frame from the videos.
 * UNPACK_FLIP_Y_WEBGL because HTML5 video element and WebGL video element have different orientation of y-axis
 */
function updateTexture()
{
  gl.bindTexture(gl.TEXTURE_2D, colorTexture);
  gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, videoElement);
  
  gl.bindTexture(gl.TEXTURE_2D, normalsTexture);
  gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, videoNormalsElement);
  
  gl.bindTexture(gl.TEXTURE_2D, depthTexture);
  gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, videoDepthElement);
  
  gl.bindTexture(gl.TEXTURE_2D, null);
}

/**
 * Start playing the video as soon as all 3 videos are loaded.
 * In continuation drawing starts.
 */
function startVideo()
{
  numberOfVideosLoaded++;
    
  if (numberOfVideosLoaded === 3)
  {
    videoElement.play();
    videoNormalsElement.play();
    videoDepthElement.play();

    setInterval(drawScene, 15);
  }
}

/**
 * Called when the video is done playing, but should never get fired because the video should loop
 */
function videoDone()
{
    //Unreached!
    //Problem: chrome doesn't loop, but this is still unreached!
}

/**
 * Draw the scene
 */
function drawScene()
{
  updateTexture();
  framesPerSecond++;
  
  //Clear canvas before drawing on it.
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
  
  //Establish perspective.
  //Field of view is 45 degrees, with a width/height ratio of 600:600.
  //Only draw objects between 0.1 units and 100 units away from the camera.
  perspectiveMatrix = makePerspective(45, 600.0/600.0, 0.1, 100.0);
  
  loadIdentity();
  mvTranslate([0.0, 0.0, -6.0]);  
  mvPushMatrix();

  gl.bindBuffer(gl.ARRAY_BUFFER, verticesBuffer);
  gl.vertexAttribPointer(vertexPositionAttribute, 3, gl.FLOAT, false, 0, 0);
  
  gl.bindBuffer(gl.ARRAY_BUFFER, verticesTextureCoordBuffer);
  gl.vertexAttribPointer(textureCoordAttribute, 2, gl.FLOAT, false, 0, 0);
  
  gl.activeTexture(gl.TEXTURE0);
  gl.bindTexture(gl.TEXTURE_2D, colorTexture);
  gl.uniform1i(gl.getUniformLocation(shaderProgram, "uSampler"), 0);
  gl.activeTexture(gl.TEXTURE1);
  gl.bindTexture(gl.TEXTURE_2D, normalsTexture);
  gl.uniform1i(gl.getUniformLocation(shaderProgram, "uSamplerNormals"), 1);
  gl.activeTexture(gl.TEXTURE2);
  gl.bindTexture(gl.TEXTURE_2D, depthTexture);
  gl.uniform1i(gl.getUniformLocation(shaderProgram, "uSamplerDepth"), 2);
  
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, verticesIndexBuffer);
  setMatrixUniforms();
  gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);

  mvPopMatrix(); 
}

/**
 * Initialize shaders that are defined in index.html;
 * the shader uses the textures for post-processig.
 */
function initShaders()
{
  var fragmentShader = getShader(gl, "shader-fs");
  var vertexShader = getShader(gl, "shader-vs");
  
  shaderProgram = gl.createProgram();
  gl.attachShader(shaderProgram, vertexShader);
  gl.attachShader(shaderProgram, fragmentShader);
  gl.linkProgram(shaderProgram);
  
  if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS))
  {
    alert("Unable to initialize the shader program.");
  }
  
  gl.useProgram(shaderProgram);
  
  vertexPositionAttribute = gl.getAttribLocation(shaderProgram, "aVertexPosition");
  gl.enableVertexAttribArray(vertexPositionAttribute);
  
  textureCoordAttribute = gl.getAttribLocation(shaderProgram, "aTextureCoord");
  gl.enableVertexAttribArray(textureCoordAttribute);
}

/**
 * Look for a script with the specified ID
 * @param {WebGLRenderingContext} gl
 * @param {string} id of element in document
 * @returns {WebGLShader|null} shader
 */
function getShader(gl, id)
{
  var shaderScript = document.getElementById(id);
  
  //Didn't find an element with the specified ID, so abort.
  if (!shaderScript)
  {
    return null;
  }
  
  //Walk through the source element's children, building the shader source string.
  var theSource = "";
  var currentChild = shaderScript.firstChild;
  while(currentChild)
  {
    if (currentChild.nodeType === 3)
    {
      theSource += currentChild.textContent;
    }
    
    currentChild = currentChild.nextSibling;
  }
  
  //Figure out the type of shader script, based on its MIME type.
  //WebGL only supports vertex-shader and fragment-shader.
  var shader;
  if (shaderScript.type === "x-shader/x-fragment")
  {
    shader = gl.createShader(gl.FRAGMENT_SHADER);
  }
  else if (shaderScript.type === "x-shader/x-vertex")
  {
    shader = gl.createShader(gl.VERTEX_SHADER);
  }
  else
  {
    //Unknown shader type
    return null;
  }
  
  gl.shaderSource(shader, theSource);
  gl.compileShader(shader);
  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS))
  {
    alert("An error occurred while compiling the shaders: " + gl.getShaderInfoLog(shader));
    return null;
  }
  
  return shader;
}

/**
 * Video playback failed, show message to user to find out why
 * @param {Event} e
 */
function videoLoadFailed(e)
{
   switch (e.target.error.code)
   {
     case e.target.error.MEDIA_ERR_ABORTED:
       alert('You aborted the video playback.');
       break;
     case e.target.error.MEDIA_ERR_NETWORK:
       alert('A network error caused the video download to fail part-way.');
       break;
     case e.target.error.MEDIA_ERR_DECODE:
       alert('The video playback was aborted due to a corruption problem or because the video used features your browser did not support (regarding decoding).');
       break;
     case e.target.error.MEDIA_ERR_SRC_NOT_SUPPORTED:
       alert('The video could not be loaded, either because the server or network failed or because the format is not supported.\nHints: Before changing video format, try a smaller filesize. When using Chrome and loading video locally, make sure to start chrome with --disable-web-security.');
       break;
     default:
       alert('An unknown error occurred.');
       break;
   }
 }

//The following are matrix utility functions

/**
 * Load idintity for model-view-matrix
 */
function loadIdentity()
{
  mvMatrix = Matrix.I(4);
}

/**
 * Multiply matrix with matrix
 * @param {Matrix} m
 */
function multMatrix(m)
{
  mvMatrix = mvMatrix.x(m);
}

/**
 * @param {Array.<number>} v
 */
function mvTranslate(v)
{
  multMatrix(Matrix.Translation($V([v[0], v[1], v[2]])).ensure4x4());
}

function setMatrixUniforms()
{
  var pUniform = gl.getUniformLocation(shaderProgram, "uPMatrix");
  gl.uniformMatrix4fv(pUniform, false, new Float32Array(perspectiveMatrix.flatten()));

  var mvUniform = gl.getUniformLocation(shaderProgram, "uMVMatrix");
  gl.uniformMatrix4fv(mvUniform, false, new Float32Array(mvMatrix.flatten()));
}

var mvMatrixStack = [];

function mvPushMatrix()
{
  mvMatrixStack.push(mvMatrix.dup());
}

function mvPopMatrix()
{
  if (!mvMatrixStack.length)
  {
    throw("Can't pop from an empty matrix stack.");
  }
  mvMatrix = mvMatrixStack.pop();
}