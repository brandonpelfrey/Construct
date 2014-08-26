/**
 * Created by brand_000 on 8/25/2014.
 */

var Construct = {};

Vec2 = function(x, y) {
    this.values = [x,y];
}

Vec2.prototype.copy = function() {
    return new Vec2(this.values[0], this.values[1]);
}

Vec2.prototype.add  = function(v) {
    this.values[0] += v.values[0];
    this.values[1] += v.values[1];
    return this;
}
Vec2.prototype.subtract = function(v) {
    this.values[0] -= v.values[0];
    this.values[1] -= v.values[1];
    return this;
}
Vec2.prototype.multiplyScalar = function(scalar) {
    this.values[0] *= scalar;
    this.values[1] *= scalar;
    return this;
}
Vec2.prototype.div = function(scalar) {
    var inverse = 1.0 / scalar;
    this.values[0] *= inverse;
    this.values[1] *= inverse;
    return this;
}
Vec2.prototype.dot = function(v) {
    return this.values[0] * v.values[0] + this.values[1] * v.values[1];
}
Vec2.prototype.norm = function() {
    return Math.sqrt(this.values[0] * this.values[0] + this.values[1] * this.values[1]);
}
Vec2.prototype.normSquared = function() {
    return this.values[0] * this.values[0] + this.values[1] * this.values[1];
}
Vec2.prototype.normalize = function() {
    var length = this.length();
    return this.div(length);
}

///////////////////////////////////////////////////////////////////////////
ScalarFieldNodes = {};

ScalarFieldNodes.ConstantScalarFieldNode = function(value) {
    this.value = value;
}
ScalarFieldNodes.ConstantScalarFieldNode.prototype = {
    evaluate: function(x) {
        return this.value;
    },
    grad: function(x) {
        return new Vec2(0,0);
    }
}

/////

ScalarFieldNodes.Addition = function(left, right) {
    this.left = left;
    this.right = right;
}
ScalarFieldNodes.Addition.prototype = {
    evaluate: function(x) {
        return this.left.evaluate(x) + this.right.evaluate(x);
    },
    grad: function(x) {
        return this.left.grad(x).add(this.right.grad(x));
    }
}

////

ScalarFieldNodes.Subtraction = function(left, right) {
    this.left = left;
    this.right = right;
}
ScalarFieldNodes.Subtraction.prototype = {
    evaluate: function(x) {
        return this.left.evaluate(x) - this.right.evaluate(x);
    },
    grad: function(x) {
        return this.left.grad(x).subtract(this.right.grad(x));
    }
}

////

ScalarFieldNodes.MultiplyScalar = function(left, right) {
    this.left = left;
    this.right = right;
}
ScalarFieldNodes.MultiplyScalar.prototype = {
    evaluate: function(x) {
        return this.left.evaluate(x) * this.right.evaluate(x);
    },
    grad: function(x) {
        return this.left.grad(x)
                .multiplyScalar(this.right.evaluate(x))
            .add(this.right.grad(x)
                .multiplyScalar(this.left.evaluate(x)));
    }
}

////

ScalarFieldNodes.DivideScalar = function(left, right) {
    this.left = left;
    this.right = right;
}
ScalarFieldNodes.DivideScalar.prototype = {
    evaluate: function(x) {
        return this.left.evaluate(x) / this.right.evaluate(x);
    },
    grad: function(x) {
        var lhs_gradient = this.left.grad(x);
        var rhs_gradient = this.right.grad(x);
        var lhs_eval = this.left.evaluate(x);
        var rhs_inverse = this.right.evaluate(x);

        // (a / b)' = b^-1 * (a' - a b' b^-1)
        return (lhs_gradient.subtract(
                    rhs_gradient.multiplyScalar(lhs_eval * rhs_inverse)
                ) ).multiplyScalar(rhs_inverse);
    }
}
////////////////////////////////////

ScalarField = function(value) {
    if (typeof value === 'number') {
        this.node = new ScalarFieldNodes.ConstantScalarFieldNode(value);
    } else {
        this.node = value;
    }
}
ScalarField.prototype.evaluate = function(x) { return this.node.evaluate(x); }
ScalarField.prototype.grad = function(x) { return this.node.grad(x); }

// Operations on ScalarFields
BinaryOp = function(TNodeContainer, TFieldNodeType) {
    return function(rhs) {
        return new TNodeContainer(
            new TFieldNodeType(
                this.node, rhs.node)
        );
    }
}

ScalarField.prototype.add = BinaryOp(ScalarField, ScalarFieldNodes.Addition);
ScalarField.prototype.subtract = BinaryOp(ScalarField, ScalarFieldNodes.Subtraction);
ScalarField.prototype.multiplyScalar = BinaryOp(ScalarField, ScalarFieldNodes.MultiplyScalar);
ScalarField.prototype.divideScalar = BinaryOp(ScalarField, ScalarFieldNodes.DivideScalar);

//////////////////////////////////////

VectorFieldNodes = {};

VectorFieldNodes.ConstantVectorFieldNode = function(value) {
    this.value = value;
}
VectorFieldNodes.ConstantVectorFieldNode.prototype = {
    evaluate: function(x) {
        return this.value;
    },
    grad: function(x) {
        // TODO
    }
}

////

VectorFieldNodes.Addition = function(left, right) {
    this.left = left;
    this.right = right;
}
VectorFieldNodes.Addition.prototype = {
    evaluate: function(x) {
        return this.left.evaluate(x).add(this.right.evaluate(x));
    },
    grad: function(x) {
        // TODO
    }
}

////

VectorFieldNodes.Subtraction = function(left, right) {
    this.left = left;
    this.right = right;
}
VectorFieldNodes.Subtraction.prototype = {
    evaluate: function(x) {
        return this.left.evaluate(x).subtract(this.right.evaluate(x));
    },
    grad: function(x) {
        // TODO
    }
}

/////

VectorFieldNodes.Identity = function() {
}
VectorFieldNodes.Identity.prototype = {
    evaluate: function(x) {
        return x.copy();
    },
    grad: function(x) {
        // TODO
    }
}

/////

VectorFieldNodes.Identity = function() {
}
VectorFieldNodes.Identity.prototype = {
    evaluate: function(x) {
        return x;
    },
    grad: function(x) {
        // TODO
    }
}


VectorField = function(value) {
    if (value instanceof Vec2) {
        this.node = new VectorFieldNodes.ConstantVectorFieldNode(value);
    } else {
        this.node = value;
    }
}
VectorField.prototype.evaluate = function(x) { return this.node.evaluate(x); }
VectorField.prototype.grad = function(x) { return this.node.grad(x); }

VectorField.prototype.add = BinaryOp(VectorField, VectorFieldNodes.Addition);
VectorField.prototype.subtract = BinaryOp(VectorField, VectorFieldNodes.Subtraction);
var identity = function(){ return new VectorField(new VectorFieldNodes.Identity()); }

///////////////////////////////////////////////////////


var s = new ScalarField(12);
var t = new ScalarField(4);
var u = new VectorField(new Vec2(1, 2));
console.log( identity().subtract(identity()).evaluate(new Vec2(1,4)) );
