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
            type: 'f',
            name: this.parameterName,
            value: 1.0
        };
    }
}

var webglParameter = function(parameterName) { return new ScalarField(new ScalarFieldNodes.WebGLParameter(parameterName));}

////

ScalarFieldNodes.WebGLGrid = function(options) {
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
        if (this._numbering === undefined) {
            console.log('getWebGLUniform was called on a node before it had been numbered!');
        }
        return {
            type: 't',
            name: 'uniform_for_node_' + this._numbering,
            value: this.texture
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
    code.push('  if( uv.x<=0.0 || uv.y<=0.0 || uv.x>=1.0 || uv.y>=1.0) return 0.0;');
    code.push('  return texture2D(' + this.getWebGLUniform().name +  ', uv).x;');
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

// It's really expensive to create render targets, so they're cached here and returned in a cycle when
// requesting "new ones". This is _terrible_.
var textureFactory = (function(){

    var _createRenderTarget = function(resolution) {
        return new THREE.WebGLRenderTarget( resolution[0], resolution[1],
            {
                minFilter:THREE.LinearFilter,
                magFilter:THREE.LinearFilter,/*
                wrapS:THREE.RenderTargetWrapping,
                wrapT:THREE.RenderTargetWrapping,*/
                format:THREE.RGBAFormat,
                stencilBuffer:false,
                depthBuffer:false,
                type:THREE.FloatType
            });
    };

    var _targets = [];
    var _currentPtr = 0;
    var _nTargets = 4;

    (function(){
        for(var i=0; i<_nTargets; ++i) {
            _targets.push(_createRenderTarget([1024, 1024]));
        }
    })();

    return {
        getRenderTarget: function(resolution) {
            var target = _targets[_currentPtr];
            _currentPtr = (_currentPtr + 1) % _nTargets;
            return target;
        }
    };
})();

var writeToGrid = function(field, options) {

    var rtTexture = textureFactory.getRenderTarget(options.resolution);

    var cameraRTT = new THREE.OrthographicCamera( -1, 1, 1, -1, -2, 2 );

    var webgl_artifacts = generate_code_webgl(field.node);

    var uniformsRTT = (function(){
        var result = {};
        for (var uniform_name in webgl_artifacts.uniforms) {
            result[uniform_name] = {
                type: webgl_artifacts.uniforms[uniform_name].type,
                value: webgl_artifacts.uniforms[uniform_name].value
            };
        }
        return result;
    })();

    uniformsRTT.resolution = { type: "v2", value: new THREE.Vector2(options.resolution[0], options.resolution[1]) };

    var materialRTT = new THREE.ShaderMaterial( {
        uniforms: uniformsRTT,
        vertexShader: document.getElementById( 'vertexShader' ).textContent,
        fragmentShader: webgl_artifacts.code
    } );

    var sceneRTT = new THREE.Scene();
    sceneRTT.add( new THREE.Mesh( new THREE.PlaneGeometry( 2, 2 ), materialRTT ) );

    var renderer = options.renderer;
    renderer.setSize( options.resolution[0], options.resolution[1] );
    renderer.setRenderTarget(rtTexture);
    renderer.clear();
    renderer.render( sceneRTT, cameraRTT, rtTexture, true );

    console.log('asd');

    return new ScalarField(new ScalarFieldNodes.WebGLGrid({
        texture: rtTexture,
        gridMin: new Vec2(-1,-1),
        gridMax: new Vec2(1,1)
    }))
}

////////////////

var number_nodes = function(root_node) {
    // Post-order traversal of the expression tree so numbering satisfies dependencies
    var _numbering = 0;
    function _visit(_node) {
        for (var i = 0; i < _node.children.length; ++i) {
            _visit(_node.children[i]);
        }
        _node._numbering = _numbering ++;
    }
    _visit(root_node);
}

var get_uniforms = function(root_node) {
    var uniforms = {};
    function _visit(_node) {
        for (var i = 0; i < _node.children.length; ++i) {
            _visit(_node.children[i]);
        }

        if (_node.getWebGLUniform) {
            var uniformParameters = _node.getWebGLUniform();
            uniforms[uniformParameters.name] = uniformParameters;
        }
    }
    _visit(root_node);

    return uniforms;
}

///////////////////////////////

var generate_code_webgl = function(root_node) {

    number_nodes(root_node);

    var uniforms = get_uniforms(root_node);

    var body_code = [], node;
    var generated_functions = {};

    // Thing to make variable names
    var namer = function(node) {
        return 'node_' + node._numbering;
    }

    // Post-order traversal of the expression tree to satisfy dependencies
    function _visit(_node) {
        for (var i = 0; i < _node.children.length; ++i) {
            _visit(_node.children[i]);
        }

        // Generate code for this node now that it and its children have numberings
        if (_node.codegen_webgl === undefined) {
            console.error('WebGL Not Implmentated for node ', _node);
        }

        if (!generated_functions.hasOwnProperty(_node._numbering)) {
            // Generate the lines of code for this node
            var node_code_lines = _node.codegen_webgl({
                function_name: namer(_node),
                function_arguments: _node.children.map(namer)
            });

            // Append each of the generated lines for this node to the final code output
            while (node_code_lines.length > 0) {
                body_code.push(node_code_lines.shift());
            }

            generated_functions[_node._numbering] = true;
        }
    }
    _visit(root_node);

    var result_code = [];

    // Push all the uniforms into the code
    for (var name in uniforms) {
        if (uniforms[name].type === 't') {
            result_code.push('uniform sampler2D ' + name + ';');
        } else if (uniforms[name].type === 'f') {
            result_code.push('uniform float ' + name + ';');
        }
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
    result_code.push('  gl_FragColor = vec4(val,val,val,1);');
    //result_code.push('  if(dot(p,p)<0.00001) { gl_FragColor = vec4(1,0,0,1); }');

    result_code.push('}');

    return {
        code: result_code.join('\n'),
        uniforms: uniforms
    };
}