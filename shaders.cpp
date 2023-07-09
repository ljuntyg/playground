#include "shaders.h"

#include <iostream>

namespace shaders
{
    // Function to compile a shader, helper method to createShaderProgram
    GLuint compileShader(const GLenum type, const GLchar *source)
    {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (!status)
        {
            GLchar infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Failed to compile shader: " << infoLog << std::endl;
        }

        return shader;
    }

    // Function to create a shader program
    GLuint createShaderProgram(const GLchar* vertexShaderSource, const GLchar* fragmentShaderSource)
    {
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (!status)
        {
            GLchar infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Failed to link shader program: " << infoLog << std::endl;

            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            return 0;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    /* 
        RENDERER/MODEL SHADERS
    */
    const GLchar* rendererVertexShaderSource = R"glsl(
        #version 330

        layout(location = 0) in vec3 aPosition;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec2 aTexCoords;

        out vec3 vViewSpacePosition;
        out vec3 vViewSpaceNormal;
        out vec2 vTexCoords;

        uniform mat4 uModelViewProjMatrix;
        uniform mat4 uModelViewMatrix;
        uniform mat4 uNormalMatrix;

        void main()
        {
            vViewSpacePosition = vec3(uModelViewMatrix * vec4(aPosition, 1));
            vViewSpaceNormal = normalize(vec3(uNormalMatrix * vec4(aNormal, 0)));
            vTexCoords = aTexCoords;
            gl_Position =  uModelViewProjMatrix * vec4(aPosition, 1);
        }
    )glsl";
    const GLchar* rendererFragmentShaderSource = R"glsl(
        #version 330

        // A reference implementation can be found here:
        // https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/master/src/shaders/metallic-roughness.frag
        // Here we implement a simpler version handling only one directional light and
        // no normal map/opacity map

        in vec3 vViewSpacePosition;
        in vec3 vViewSpaceNormal;
        in vec2 vTexCoords;
        // Here we use vTexCoords but we should use vTexCoords1 or vTexCoords2 depending
        // on the material because glTF can handle two texture coordinates sets per
        // object see
        // https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/master/src/shaders/textures.glsl
        // for a reference implementation

        uniform vec3 uLightDirection;
        uniform vec3 uLightIntensity;

        uniform vec4 uBaseColorFactor;
        uniform float uMetallicFactor;
        uniform float uRoughnessFactor;
        uniform vec3 uEmissiveFactor;
        uniform float uOcclusionStrength;

        uniform sampler2D uBaseColorTexture;
        uniform sampler2D uMetallicRoughnessTexture;
        uniform sampler2D uEmissiveTexture;
        uniform sampler2D uOcclusionTexture;

        uniform int uApplyOcclusion;

        out vec3 fColor;

        // Constants
        const float GAMMA = 2.2;
        const float INV_GAMMA = 1. / GAMMA;
        const float M_PI = 3.141592653589793;
        const float M_1_PI = 1.0 / M_PI;

        // We need some simple tone mapping functions
        // Basic gamma = 2.2 implementation
        // Stolen here:
        // https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/master/src/shaders/tonemapping.glsl

        // linear to sRGB approximation
        // see http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
        vec3 LINEARtoSRGB(vec3 color) { return pow(color, vec3(INV_GAMMA)); }

        // sRGB to linear approximation
        // see http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
        vec4 SRGBtoLINEAR(vec4 srgbIn)
        {
        return vec4(pow(srgbIn.xyz, vec3(GAMMA)), srgbIn.w);
        }

        // The model is mathematically described here
        // https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#appendix-b-brdf-implementation
        // We try to use the same or similar names for variables
        // One thing that is not descibed in the documentation is that the BRDF value
        // "f" must be multiplied by NdotL at the end.
        void main()
        {
        vec3 N = normalize(vViewSpaceNormal);
        vec3 V = normalize(-vViewSpacePosition);
        vec3 L = uLightDirection;
        vec3 H = normalize(L + V);

        vec4 baseColorFromTexture =
            SRGBtoLINEAR(texture(uBaseColorTexture, vTexCoords));
        vec4 metallicRougnessFromTexture =
            texture(uMetallicRoughnessTexture, vTexCoords);

        vec4 baseColor = uBaseColorFactor * baseColorFromTexture;
        vec3 metallic = vec3(uMetallicFactor * metallicRougnessFromTexture.b);
        float roughness = uRoughnessFactor * metallicRougnessFromTexture.g;

        // https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#pbrmetallicroughnessmetallicroughnesstexture
        // "The metallic-roughness texture.The metalness values are sampled from the B
        // channel.The roughness values are sampled from the G channel."

        vec3 dielectricSpecular = vec3(0.04);
        vec3 black = vec3(0.);

        vec3 c_diff =
            mix(baseColor.rgb * (1 - dielectricSpecular.r), black, metallic);
        vec3 F_0 = mix(vec3(dielectricSpecular), baseColor.rgb, metallic);
        float alpha = roughness * roughness;

        float VdotH = clamp(dot(V, H), 0., 1.);
        float baseShlickFactor = 1 - VdotH;
        float shlickFactor = baseShlickFactor * baseShlickFactor; // power 2
        shlickFactor *= shlickFactor;                             // power 4
        shlickFactor *= baseShlickFactor;                         // power 5
        vec3 F = F_0 + (vec3(1) - F_0) * shlickFactor;

        float sqrAlpha = alpha * alpha;
        float NdotL = clamp(dot(N, L), 0., 1.);
        float NdotV = clamp(dot(N, V), 0., 1.);
        float visDenominator =
            NdotL * sqrt(NdotV * NdotV * (1 - sqrAlpha) + sqrAlpha) +
            NdotV * sqrt(NdotL * NdotL * (1 - sqrAlpha) + sqrAlpha);
        float Vis = visDenominator > 0. ? 0.5 / visDenominator : 0.0;

        float NdotH = clamp(dot(N, H), 0., 1.);
        float baseDenomD = (NdotH * NdotH * (sqrAlpha - 1.) + 1.);
        float D = M_1_PI * sqrAlpha / (baseDenomD * baseDenomD);

        vec3 f_specular = F * Vis * D;

        vec3 diffuse = c_diff * M_1_PI;

        vec3 f_diffuse = (1. - F) * diffuse;
        vec3 emissive = SRGBtoLINEAR(texture2D(uEmissiveTexture, vTexCoords)).rgb *
                        uEmissiveFactor;

        vec3 color = (f_diffuse + f_specular) * uLightIntensity * NdotL;
        color += emissive;

        if (1 == uApplyOcclusion) {
            float ao = texture2D(uOcclusionTexture, vTexCoords).r;
            color = mix(color, color * ao, uOcclusionStrength);
        }

        fColor = LINEARtoSRGB(color);
        }
    )glsl";

    /* 
        SKYBOX SHADERS
    */
    const GLchar* skyboxVertexShaderSource = R"glsl(
        #version 330 core
        layout (location = 0) in vec3 aPos;

        out vec3 texCoords;

        uniform mat4 projection;
        uniform mat4 view;

        void main()
        {
            vec4 pos = projection * view * vec4(aPos, 1.0f);
            // Having z equal w will always result in a depth of 1.0f
            gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
            // We want to flip the z axis due to the different coordinate systems (left hand vs right hand)
            texCoords = vec3(aPos.x, aPos.y, -aPos.z);
        }    
    )glsl";
    const GLchar* skyboxFragmentShaderSource = R"glsl(
        #version 330 core
        out vec4 FragColor;

        in vec3 texCoords;

        uniform samplerCube skybox;

        void main()
        {    
            FragColor = texture(skybox, texCoords);
        }
    )glsl";

    /* 
        GUI SHADERS
    */
    const GLchar* guiVertexShaderSource = R"glsl(
        #version 330 core
        layout (location = 0) in vec3 aPos;

        uniform mat4 model;
        uniform mat4 projection;

        out vec2 vPos;

        void main()
        {
            gl_Position = projection * model * vec4(aPos, 1.0);
            vPos = aPos.xy;
        }
    )glsl";
    const GLchar* guiFragmentShaderSource = R"glsl(
        #version 330 core
        in vec2 vPos;

        out vec4 FragColor;

        uniform vec4 color;
        uniform vec2 resolution;  // Width and height of the UI element
        uniform float cornerRadius;  // Radius of the corners in pixels

        void main()
        {
            // Compute the position in the UI element in pixels
            vec2 pos = vPos * resolution;

            // Define the light source position at the top right corner
            vec2 lightSource = vec2(1.0, 1.0) * resolution;

            // Compute the distance from the light source to the current pixel
            float dist = distance(lightSource, pos);

            // Normalize the distance based on the size of the UI element,
            // so that it ranges from 0.0 at the light source to 1.0 at the corner
            float normDist = dist / (sqrt(2.0) * length(lightSource));

            vec4 startColor = vec4(color.rgb * 1.3, 1.0);
            vec4 endColor = vec4(color.rgb * 0.9, 1.0);

            // Create a smoother gradient by using smoothstep
            vec4 gradientColor = mix(startColor, endColor, smoothstep(0.0, 1.0, normDist));

            // Calculate distance to each corner and keep the minimum distance
            vec2 d = abs(pos - resolution*0.5) - (resolution*0.5 - vec2(cornerRadius));
            float distToCorner = length(max(d, 0.0));

            // Blend the color based on the distance to the corner
            FragColor = mix(gradientColor, vec4(0.0), smoothstep(0.0, 1.0, max(0.0, distToCorner) / cornerRadius));
        }
    )glsl";

    /* 
        GUI TEXT SHADERS
    */
    const GLchar* textVertexShaderSource = R"glsl(
        #version 330 core
        layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
        out vec2 TexCoords;

        uniform mat4 projection;
        uniform mat4 model;

        void main()
        {
            gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
            TexCoords = vertex.zw;
        }
    )glsl";
    const GLchar* textFragmentShaderSource = R"glsl(
        #version 330 core
        in vec2 TexCoords;
        out vec4 color;

        uniform sampler2D text;
        uniform vec4 textColor;
        
        const float smoothing = 1.0f;

        float median(float r, float g, float b) 
        {
            return max(min(r, g), min(max(r, g), b));
        }

        void main()
        {    
            vec3 sample = texture(text, TexCoords).rgb;
            float sigDist = median(sample.r, sample.g, sample.b) - 0.5;
            float alpha = clamp(sigDist/(fwidth(sigDist) * smoothing) + 0.5, 0.0, 1.0);
            color = vec4(textColor.rgb, textColor.a * alpha);
        }
    )glsl";
}