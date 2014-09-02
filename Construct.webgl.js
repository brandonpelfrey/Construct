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

var generate_code_webgl = function(root_node) {

    var result_code = [], key, node;
    result_code.push('uniform vec2 resolution;');

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

        var node_code_lines = _node.codegen_webgl({
            function_name: namer(_node),
            function_arguments: _node.children.map(namer)
        });

        // Append each of the generated lines for this node to the final code output
        while(node_code_lines.length > 0) {
            result_code.push(node_code_lines.shift());
        }
    }
    _visit(root_node);

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