layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

uniform mat3 u_Transform;
uniform mat3 u_View;

out vec2 POS;
out vec2 UV;

void main() {

    mat3 viewTransform = u_Transform * u_View;
	gl_Position.w = 1.0;

    gl_Position.xyz = vec3( -1.0, -1.0, 1.0) * viewTransform;
    POS = gl_Position.xy;
    UV = vec2(0.0,0.0);
    EmitVertex();

    gl_Position.xyz = vec3( 1.0, -1.0, 1.0) * viewTransform;
    POS = gl_Position.xy;
    UV = vec2(1.0,0.0);
    EmitVertex();

    gl_Position.xyz = vec3( -1.0, 1.0, 1.0) * viewTransform;
    POS = gl_Position.xy;
    UV = vec2(0.0,1.0);
    EmitVertex();

	EndPrimitive();

    gl_Position.xyz = vec3( -1.0, 1.0, 1.0) * viewTransform;
    POS = gl_Position.xy;
    UV = vec2(0.0,1.0);
    EmitVertex();

    gl_Position.xyz = vec3( 1.0, -1.0, 1.0) * viewTransform;
    POS = gl_Position.xy;
    UV = vec2(1.0,0.0);
    EmitVertex();

    gl_Position.xyz = vec3( 1.0, 1.0, 1.0) * viewTransform;
    POS = gl_Position.xy;
    UV = vec2(1.0,1.0);
    EmitVertex();
    
    EndPrimitive();
}  