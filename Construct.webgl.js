/**
 * Created by brandon pelfrey on 9/1/2014.
 */

// WebGL shader generation code for the Construct Field Nodes.

/**
 * {
 *   function_name: [string],
 *   function_arguments: [array of strings : variable names]
 * }
 */


////

ScalarFieldNodes.WebGLParameter = function(parameterName) {
    this.parameterName = parameterName;
    this.children = [];
}
ScalarFieldNodes.WebGLParameter.prototype = {
    evaluate: function(x) {
        console.error('WebGLParameter not usable in JS');
    },
    grad: function(x) {
        console.error('WebGLParameter not usable in JS');
    },
    getWebGLUniform: function() {
        return {
            type: 'float',
            name: this.parameterName
        };
    }
}

var webglParameter = function(parameterName) { return new ScalarField(new ScalarFieldNodes.WebGLParameter(parameterName));}

////

ScalarFieldNodes.WebGLGrid = function(options) {
    this.textureName = options.textureName;
    this.texture = options.texture;
    this.gridMin = options.gridMin;
    this.gridMax = options.gridMax;
    this.children = [];
}
ScalarFieldNodes.WebGLGrid.prototype = {
    evaluate: function(x) {
        console.error('WebGLGrid not usable in JS');
    },
    grad: function(x) {
        console.error('WebGLGrid not usable in JS');
    },
    getWebGLUniform: function() {
        return {
            type: 'sampler2D',
            name: this.textureName
        };
    }
}

////////////////

ScalarFieldNodes.ConstantScalarFieldNode.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('float ' + options.function_name + '(const vec2 x) {');
    code.push('  return float(' + this.value + ');');
    code.push('}');
    return code;
}

ScalarFieldNodes.InnerProduct.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('float ' + options.function_name + '(const vec2 x) {');
    code.push('  return dot(' + options.function_arguments[0] + '(x), ' + options.function_arguments[1] + '(x));');
    code.push('}');
    return code;
}

ScalarFieldNodes.Addition.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('float ' + options.function_name + '(const vec2 x) {');
    code.push('  return ' + options.function_arguments[0] + '(x) + ' + options.function_arguments[1] + '(x);');
    code.push('}');
    return code;
}

ScalarFieldNodes.Subtraction.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('float ' + options.function_name + '(const vec2 x) {');
    code.push('  return ' + options.function_arguments[0] + '(x) - ' + options.function_arguments[1] + '(x);');
    code.push('}');
    return code;
}

ScalarFieldNodes.MultiplyScalar.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('float ' + options.function_name + '(const vec2 x) {');
    code.push('  return ' + options.function_arguments[0] + '(x) * ' + options.function_arguments[1] + '(x);');
    code.push('}');
    return code;
}

ScalarFieldNodes.DivideScalar.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('float ' + options.function_name + '(const vec2 x) {');
    code.push('  return ' + options.function_arguments[0] + '(x) / ' + options.function_arguments[1] + '(x);');
    code.push('}');
    return code;
}

ScalarFieldNodes.Cosine.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('float ' + options.function_name + '(const vec2 x) {');
    code.push('  return cos(' + options.function_arguments[0] + '(x) );');
    code.push('}');
    return code;
}

ScalarFieldNodes.VectorNorm.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('float ' + options.function_name + '(const vec2 x) {');
    code.push('  return length(' + options.function_arguments[0] + '(x) );');
    code.push('}');
    return code;
}

ScalarFieldNodes.Warp.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('float ' + options.function_name + '(const vec2 x) {');
    code.push('  return ' + options.function_arguments[0] + '( ' + options.function_arguments[1] + '(x) );');
    code.push('}');
    return code;
}

ScalarFieldNodes.Mask.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('float ' + options.function_name + '(const vec2 x) {');
    code.push('  return ' + options.function_arguments[0] + '(x) > 0.0 ? 1.0 : 0.0;');
    code.push('}');
    return code;
}

ScalarFieldNodes.WebGLParameter.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('float ' + options.function_name + '(const vec2 x) {');
    code.push('  return float(' + this.parameterName +  ');');
    code.push('}');
    return code;
}

ScalarFieldNodes.WebGLGrid.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('float ' + options.function_name + '(const vec2 x) {');
    code.push('  vec2 gridMin = vec2(' + this.gridMin.values[0] + ', ' + this.gridMin.values[1] + ');');
    code.push('  vec2 gridMax = vec2(' + this.gridMax.values[0] + ', ' + this.gridMax.values[1] + ');');
    code.push('  vec2 uv = (x - gridMin) / (gridMax - gridMin);');
    code.push('  return texture2D(' + this.textureName +  ', uv).x;');
    code.push('}');
    return code;
}

/////////////////
/////////////////

VectorFieldNodes.ConstantVectorFieldNode.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('vec2 ' + options.function_name + '(const vec2 x) {');
    code.push('  return vec2(' + this.value.values[0] + ', ' + this.value.values[1] + ');');
    code.push('}');
    return code;
}

VectorFieldNodes.Identity.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('vec2 ' + options.function_name + '(const vec2 x) {');
    code.push('  return x;');
    code.push('}');
    return code;
}

VectorFieldNodes.Addition.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('vec2 ' + options.function_name + '(const vec2 x) {');
    code.push('  return ' + options.function_arguments[0] + '(x) + ' + options.function_arguments[1] + '(x);');
    code.push('}');
    return code;
}

VectorFieldNodes.Subtraction.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('vec2 ' + options.function_name + '(const vec2 x) {');
    code.push('  return ' + options.function_arguments[0] + '(x) - ' + options.function_arguments[1] + '(x);');
    code.push('}');
    return code;
}

VectorFieldNodes.MultiplyScalar.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('vec2 ' + options.function_name + '(const vec2 x) {');
    code.push('  return ' + options.function_arguments[0] + '(x) * ' + options.function_arguments[1] + '(x);');
    code.push('}');
    return code;
}

VectorFieldNodes.DivideScalar.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('vec2 ' + options.function_name + '(const vec2 x) {');
    code.push('  return ' + options.function_arguments[0] + '(x) / ' + options.function_arguments[1] + '(x);');
    code.push('}');
    return code;
}

VectorFieldNodes.Warp.prototype.codegen_webgl = function(options) {
    var code = [];
    code.push('vec2 ' + options.function_name + '(const vec2 x) {');
    code.push('  return ' + options.function_arguments[0] + '( ' + options.function_arguments[1] + '(x) );');
    code.push('}');
    return code;
}

////////////////

var writeToGrid = function(node, options) {
    // options.gridextends
    // options.resolution
    // options.outside_field

    // 1) Generate a texture of size options.resolution
    // IMPORTANT: Unclamped GL_FLOAT, single channel for scalar field

    // 2) Generate a render target and appropriate scene to render to

    // 3) render using the usual codegen_webgl( node )

    // 4) Return a new ScalarFieldNode wrapping the texture

    // *) The returned SFNode must define a uniform textureSampler
    //    (And also somehow cause that texture to bind, and there could be multiple...)

    var sceneRTT = new THREE.Scene();
    var rtTexture = new THREE.WebGLRenderTarget(
        options.resolution[0],
        options.resolution[1],
        {
            minFilter: THREE.LinearFilter,
            magFilter: THREE.LinearFilter,
            format: THREE.LuminanceFormat, // Change for Vec2
            type: THREE.FloatType // Need this for unclamped value range
        });

    var cameraRTT = new THREE.OrthographicCamera( window.innerWidth / - 2, window.innerWidth / 2, window.innerHeight / 2, window.innerHeight / - 2, -10000, 10000 );
    cameraRTT.position.z = 100;

    // TODO: Setup uniforms from node and get node generated code

    var material = new THREE.ShaderMaterial( {
        uniforms: { time: { type: "f", value: 0.0 } },
        vertexShader: document.getElementById( 'vertexShader' ).textContent,
        fragmentShader: document.getElementById( 'fragment_shader_pass_1' ).textContent
    } );

    var plane = new THREE.PlaneGeometry( 2, 2 );
    var quad = new THREE.Mesh( plane, material );
    quad.position.z = 1;
    sceneRTT.add( quad );

    var renderer = new THREE.WebGLRenderer();
    renderer.setSize( options.resolution[0], options.resolution[1] );
    renderer.autoClear = false;
    renderer.clear();
    renderer.render( sceneRTT, cameraRTT, rtTexture, true );

    return new ScalarField(new ScalarFieldNodes.WebGLGrid({
        // TODO: Fill in after removing textureName from WebGLGrid
    }))
}

////////////////

var generate_code_webgl = function(root_node) {

    var body_code = [], key, node;
    var uniforms = {};

    // Thing to make variable names
    var namer = function(node) {
        return 'node_' + node._numbering;
    }

    // Post-order traversal of the expression tree to satisfy dependencies
    var _numbering = 0;
    function _visit(_node) {
        for (var i = 0; i < _node.children.length; ++i) {
            _visit(_node.children[i]);
        }
        _node._numbering = _numbering ++;

        // Generate code for this node now that it and its children have numberings
        if (_node.codegen_webgl === undefined) {
            console.error('WebGL Not Implmentated for node ', _node);
        }

        if (_node.getWebGLUniform) {
            var uniformParameters = _node.getWebGLUniform();
            uniforms[uniformParameters.name] = uniformParameters;
        }

        var node_code_lines = _node.codegen_webgl({
            function_name: namer(_node),
            function_arguments: _node.children.map(namer)
        });

        // Append each of the generated lines for this node to the final code output
        while(node_code_lines.length > 0) {
            body_code.push(node_code_lines.shift());
        }
    }
    _visit(root_node);

    var result_code = [];

    // Push all the uniforms into the code
    for (var name in uniforms) {
        result_code.push('uniform ' + uniforms[name].type + ' ' + name + ';');
    }
    result_code.push('uniform vec2 resolution;');

    // Push body
    for (var i = 0; i < body_code.length; ++i) {
        result_code.push(body_code[i]);
    }

    // Push main rendering code
    result_code.push('void main() {');
    result_code.push('  vec2 p = -1.0 + 2.0 * gl_FragCoord.xy / resolution.xy;');

    // The top-most node determine the return type.
    // TODO: For now, just stupidly assuming we're always generating code for a scalar field
    result_code.push('  float val = ' + namer(root_node) + '(p);');
    result_code.push('  gl_FragColor = vec4(val,val,val,val);');
    result_code.push('  if(dot(p,p)<0.00001) { gl_FragColor = vec4(1,0,0,1); }');

    result_code.push('}');

    return result_code.join('\n');
}