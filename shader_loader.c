#include "shader_loader.h"

unsigned int sl_compile_and_link_shaders(char* vertex_shader_source_path, char* fragment_shader_source_path) {
    //index 0 -> vertex shader, index 1 -> fragment_shader
    char * paths[] = {vertex_shader_source_path, fragment_shader_source_path};
    const char * const names[] = {"vertex", "fragment"};    
    const unsigned int shader_types[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
    
    unsigned int shader_ids[2] = {0};        
    unsigned int shader_program = -1;

    for(int i = 0; i < 2; i++) {
        FILE* shader_source_file = fopen(paths[i], "r");

        if(!shader_source_file) {
            fprintf(stderr, "Can't open %s shader source file.\n", names[i]);
            goto dispose;        
        }

        //Get filesize
        fseek(shader_source_file, 0L, SEEK_END);
        unsigned int file_size = ftell(shader_source_file);
        fseek(shader_source_file, 0L, SEEK_SET);

        char * shader_source = calloc(file_size + 1, 1); //Alocate memory for data from file
        
        char rc;
        unsigned int shader_source_index = 0; //index of character read

        while((rc = fgetc(shader_source_file)) != EOF)
            shader_source[shader_source_index++] = rc;
        
        shader_source[shader_source_index] = 0;
        fclose(shader_source_file); //closing the file

        unsigned int shader_id = glCreateShader(shader_types[i]);
        glShaderSource(shader_id, 1, (const char * const*)&shader_source, NULL);
        glCompileShader(shader_id);

        free(shader_source); //free alocated memory

        int shader_compilation_status;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &shader_compilation_status); //compilation success status
        if(!shader_compilation_status) { //check for bad compilation
            char compilation_info[1024] = {0};
            glGetShaderInfoLog(shader_id, 1023, NULL, compilation_info);
            fprintf(stderr, "%s shader compilation failed: -> %s\n", names[i], compilation_info);
            goto dispose;
        }
        shader_ids[i] = shader_id;    
    } 
    
    //linking shaders into a shader program
    shader_program = glCreateProgram();
    for(int i = 0; i < 2; i++) {
        glAttachShader(shader_program, shader_ids[i]);
    }
    glLinkProgram(shader_program);

    int shader_linking_status;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &shader_linking_status);
    if(!shader_linking_status) { //check for bad linking
        char linking_info[1024] = {0};
        glGetProgramInfoLog(shader_program, 1023, NULL, linking_info);
        fprintf(stderr, "Shader linking failed: -> %s\n", linking_info);
    }
    
    dispose:
    for(int i = 0; i < 2; i++) {
        if(!shader_ids[i])
            break;
        glDeleteShader(shader_ids[i]);
    }    
    return shader_program;        
}

