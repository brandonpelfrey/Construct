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
Vec2.prototype.minus = function(v) {
    this.values[0] -= v.values[0];
    this.values[1] -= v.values[1];
    return this;
}
Vec2.prototype.multiplyScalar = function(scalar) {
    this.values[0] *= scalar;
    this.values[1] *= scalar;
    return this;
}
Vec2.prototype.divideScalar = function(scalar) {
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
    return this.divideScalar(length);
}

///////////////////////////////////////////////////////////////////////////

/** values is row-major ordered array of the elements of a 2x2 Real-valued matrix. */
Mat2 = function(values) {
    this.values = values;
}
Mat2.prototype.copy = function() {
    return new Mat2(this.values.slice());
}
Mat2.identity = function() {
    return new Mat2([1,0,0,1]);
}
Mat2.zero = function() {
    return new Mat2([0,0,0,0]);
}
Mat2.prototype.add = function(rhs) {
    for(var i=0; i<4; ++i) {
        this.values[i] += rhs.values[i];
    }
    return this;
}
Mat2.prototype.minus = function(rhs) {
    for(var i=0; i<4; ++i) {
        this.values[i] -= rhs.values[i];
    }
    return this;
}
Mat2.prototype.multiplyScalar = function(scalar) {
    for(var i=0; i<4; ++i) {
        this.values[i] *= scalar;
    }
    return this;
}
Mat2.prototype.multiplyMat2 = function(rhs) {
    var a = this.values[0] * rhs.values[0] + this.values[1] * this.values[2];
    var b = this.values[0] * rhs.values[1] + this.values[1] * this.values[3];
    var c = this.values[2] * rhs.values[0] + this.values[3] * this.values[2];
    var d = this.values[2] * rhs.values[1] + this.values[3] * this.values[3];
    this.values = [a, b, c, d];
    return this;
}

/** Note, does not modify Mat2, returns new Vec2. */
Mat2.prototype.multiplyVec2 = function(v) {
    return new Vec2(this.values[0] * v.values[0] + this.values[1] * v.values[1],
        this.values[2] * v.values[0] + this.values[3] * v.values[1]);
}
Mat2.prototype.divideScalar = function(scalar) {
    var inverse = 1.0 / scalar;
    for(var i=0; i<4; ++i) {
        this.values[i] *= inverse;
    }
    return this;
}
Mat2.prototype.transpose = function(){
    var temp = this.values[1];
    this.values[1] = this.values[2];
    this.values[2] = temp;
    return this;
}

///////////////////////////////////////////////////////////////////////////
ScalarFieldNodes = {};

ScalarFieldNodes.ConstantScalarFieldNode = function(value) {
    this.value = value;
    this.children = [];
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
    this.children = [left, right];
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
    this.children = [left, right];
}
ScalarFieldNodes.Subtraction.prototype = {
    evaluate: function(x) {
        return this.left.evaluate(x) - this.right.evaluate(x);
    },
    grad: function(x) {
        return this.left.grad(x).minus(this.right.grad(x));
    }
}

////

ScalarFieldNodes.MultiplyScalar = function(left, right) {
    this.left = left;
    this.right = right;
    this.children = [left, right];
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
    this.children = [left, right];
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
        return (lhs_gradient.minus(
                    rhs_gradient.multiplyScalar(lhs_eval * rhs_inverse)
                ) ).multiplyScalar(rhs_inverse);
    }
}

////

// Function composition : warp(f,g)(x) == f(g(x))
ScalarFieldNodes.Warp = function(left, right) {
    this.left = left;
    this.right = right;
    this.children = [left, right];
}
ScalarFieldNodes.Warp.prototype = {
    evaluate: function(x) {
        return this.left.evaluate(this.right.evaluate(x));
    },
    grad: function(x) {
        // grad f(g(x)) == grad(g)(x) * grad(f)(g(x))
        return this.right.grad(x).multiplyVec2(
            this.left.grad(x).multiplyVec2(this.right.evaluate(x))
        );
    }
}

////

ScalarFieldNodes.InnerProduct = function(left, right) {
    this.left = left;
    this.right = right;
    this.children = [left, right];
}
ScalarFieldNodes.InnerProduct.prototype = {
    evaluate: function(x) {
        return this.left.evaluate(x).dot(this.right.evaluate(x));
    },
    grad: function(x) {
        // TODO : Check, Should these gradients be transposed?
        return this.left.grad(x).multiplyVec2(this.right.evaluate(x))
            .add(this.right.grad(x).multiplyVec2(this.left.evaluate(x)))
    }
}

////

ScalarFieldNodes.VectorNorm = function(vectorFieldNode) {
    this.vectorFieldNode = vectorFieldNode;
    this.children = [vectorFieldNode];
}
ScalarFieldNodes.VectorNorm.prototype = {
    evaluate: function(x) {
        return this.vectorFieldNode.evaluate(x).norm();
    },
    grad: function(x) {
        var v_eval = this.vectorFieldNode.evaluate(x);
        var v_grad = this.vectorFieldNode.grad(x);
        // TODO : Again... Transpose? Need to check.
        return v_grad.transpose().multiplyVec2(v_eval).divideScalar(v_eval.norm());
    }
}

////

ScalarFieldNodes.Cosine = function(scalarFieldNode) {
    this.scalarFieldNode = scalarFieldNode;
    this.children = [scalarFieldNode];
}
ScalarFieldNodes.Cosine.prototype = {
    evaluate: function(x) {
        return Math.cos(this.scalarFieldNode.evaluate(x));
    },
    grad: function(x) {
        var sin_eval = Math.sin(this.scalarFieldNode.evaluate(x));
        var v_grad = this.scalarFieldNode.grad(x);
        // TODO : Again... Transpose? Need to check.
        return v_grad.multiplyScalar(sin_eval);
    }
}

////

ScalarFieldNodes.Mask = function(scalarFieldNode) {
    this.scalarFieldNode = scalarFieldNode;
    this.children = [scalarFieldNode];
}
ScalarFieldNodes.Mask.prototype = {
    evaluate: function(x) {
        return this.vectorFieldNode.evaluate(x) > 1 ? 1.0 : 0.0;
    },
    grad: function(x) {
        // Everywhere but at the boundary of this function the gradient is zero
        // (And infinite at the boundary)
        return new Vec2(0.0,0.0);
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
UnaryOp = function(TNodeContainer, TFieldNodeType) {
    return function() {
        return new TNodeContainer(
            new TFieldNodeType(
                this.node)
        );
    }
}

BinaryOp = function(TNodeContainer, TFieldNodeType) {
    return function(rhs) {
        return new TNodeContainer(
            new TFieldNodeType(
                this.node, rhs.node)
        );
    }
}

ScalarField.prototype.add = BinaryOp(ScalarField, ScalarFieldNodes.Addition);
ScalarField.prototype.minus = BinaryOp(ScalarField, ScalarFieldNodes.Subtraction);
ScalarField.prototype.multiplyScalar = BinaryOp(ScalarField, ScalarFieldNodes.MultiplyScalar);
ScalarField.prototype.divideScalar = BinaryOp(ScalarField, ScalarFieldNodes.DivideScalar);
ScalarField.prototype.warp = BinaryOp(ScalarField, ScalarFieldNodes.Warp);
ScalarField.prototype.cos = UnaryOp(ScalarField, ScalarFieldNodes.Cosine);

//////////////////////////////////////

VectorFieldNodes = {};

VectorFieldNodes.ConstantVectorFieldNode = function(value) {
    this.value = value;
    this.children = [];
}
VectorFieldNodes.ConstantVectorFieldNode.prototype = {
    evaluate: function(x) {
        return this.value;
    },
    grad: function(x) {
        return Mat2.zero();
    }
}

////

VectorFieldNodes.Addition = function(left, right) {
    this.left = left;
    this.right = right;
    this.children = [left, right];
}
VectorFieldNodes.Addition.prototype = {
    evaluate: function(x) {
        return this.left.evaluate(x).add(this.right.evaluate(x));
    },
    grad: function(x) {
        return this.left.grad(x).add(this.right.grad(x));
    }
}

////

VectorFieldNodes.Subtraction = function(left, right) {
    this.left = left;
    this.right = right;
    this.children = [left, right];
}
VectorFieldNodes.Subtraction.prototype = {
    evaluate: function(x) {
        return this.left.evaluate(x).minus(this.right.evaluate(x));
    },
    grad: function(x) {
        return this.left.grad(x).minus(this.right.grad(x));
    }
}

/////

VectorFieldNodes.MultiplyScalar = function(left, right) {
    this.left = left;
    this.right = right;
    this.children = [left, right];
}
VectorFieldNodes.MultiplyScalar.prototype = {
    evaluate: function(x) {
        return this.left.evaluate(x).multiplyScalar(this.right.evaluate(x));
    },
    grad: function(x) {
        //TODO
    }
}

////

VectorFieldNodes.DivideScalar = function(left, right) {
    this.left = left;
    this.right = right;
    this.children = [left, right];
}
VectorFieldNodes.DivideScalar.prototype = {
    evaluate: function(x) {
        return this.left.evaluate(x).divideScalar(this.right.evaluate(x));
    },
    grad: function(x) {
        //TODO
    }
}

////

VectorFieldNodes.Identity = function() {
    this.children = [];
}
VectorFieldNodes.Identity.prototype = {
    evaluate: function(x) {
        return x.copy();
    },
    grad: function(x) {
        return Mat2.identity();
    }
}

/////

// Function composition : warp(f,g)(x) == f(g(x))
VectorFieldNodes.Warp = function(left, right) {
    this.left = left;
    this.right = right;
    this.children = [left, right];
}
VectorFieldNodes.Warp.prototype = {
    evaluate: function(x) {
        return this.left.evaluate(this.right.evaluate(x));
    },
    grad: function(x) {
        // grad f(g(x)) == grad(g)(x) * grad(f)(g(x))
        return this.right.grad(x).multiplyMat2(
            this.left.grad(x).multiplyVec2(this.right.evaluate(x))
        );
    }
}

////

VectorFieldNodes.Grad = function(scalarFieldNode) {
    this.node = scalarFieldNode;
    this.children = [scalarFieldNode];
}
VectorFieldNodes.Grad.prototype = {
    evaluate: function(x) {
        return this.node.grad(x);
    },
    grad: function(x) {
        console.error('Cannot take second derivatives in the Construct');
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
VectorField.prototype.minus = BinaryOp(VectorField, VectorFieldNodes.Subtraction);
VectorField.prototype.multiplyScalar = BinaryOp(VectorField, VectorFieldNodes.MultiplyScalar);
VectorField.prototype.divideScalar = BinaryOp(VectorField, VectorFieldNodes.DivideScalar);
VectorField.prototype.dot = BinaryOp(ScalarField, ScalarFieldNodes.InnerProduct);
VectorField.prototype.length = UnaryOp(ScalarField, ScalarFieldNodes.VectorNorm);
VectorField.prototype.warp = BinaryOp(VectorField, VectorFieldNodes.Warp);

ScalarField.prototype.grad = UnaryOp(VectorField, VectorFieldNodes.Grad);
ScalarField.prototype.mask = UnaryOp(ScalarField, ScalarFieldNodes.Mask);

///////////////////////////////////////////////////////

// Library Convenience Functions

var identity = function(){ return new VectorField(new VectorFieldNodes.Identity()); }
var dot = function(left, right) { return left.dot(right); }
var length = function(vf) { return vf.length(); }
var warp = function(field, vector_field) { return field.warp(vector_field); }
var grad = function(field) { return field.grad(); }
var mask = function(field) { return field.mask(); }
var cos = function(field) { return field.cos(); }

///////////////////////////////////////////////////////
