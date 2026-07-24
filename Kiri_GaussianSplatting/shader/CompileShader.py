import os
import sys
import argparse
from pathlib import Path


g_shader_folder = ""

def process_file(file_path: str, included_files: set) -> str:
    global g_shader_folder

    output_lines = []
    
    if file_path in included_files:
        return ""
    
    included_files.add(file_path)
    print(f"Processing file: {file_path}")
    with open(file_path, "r", encoding="utf-8") as f:
        for line in f:
            stripped = line.strip()
            
            if stripped.startswith("#include") and (stripped.endswith('.glsl"') or stripped.endswith('.metal"')):
                parts = stripped.split()
                if len(parts) >= 2:
                    inc_path_str = parts[1].strip('"<>')
                    inc_file_path = os.path.join(g_shader_folder, inc_path_str)
                    print(f"Including file: {inc_file_path}")
                    # recursive call to process the included file
                    included_content = process_file(inc_file_path, included_files)
                    output_lines.append(included_content)
                    print(f"Finished including file: {inc_file_path}")
            else:
                output_lines.append(line)


    return "".join(output_lines)




def build_vert(shader_name: str) -> str:
    global g_shader_folder

    print(f"Building vertex shader: {shader_name}.vert...")
    main_vert_path = os.path.join(g_shader_folder, f"{shader_name}.vert")
    included_files = set()
    vert_shader_code = process_file(main_vert_path, included_files)

    print(f"Vertex shader built successfully: {shader_name}.vert")

    return  vert_shader_code

def build_frag(shader_name: str) -> str:
    global g_shader_folder
    print(f"Building fragment shader: {shader_name}.frag...")

    main_frag_path = os.path.join(g_shader_folder, f"{shader_name}.frag")
    included_files = set()
    frag_shader_code = process_file(main_frag_path, included_files)

    print(f"Fragment shader built successfully: {shader_name}.frag")

    return frag_shader_code

def build_compute() -> str:
    global g_shader_folder

    print("Building compute shader...")
    main_compute_path = os.path.join(g_shader_folder, "main.comp")
    included_files = set()
    compute_shader_code = process_file(main_compute_path, included_files)

    print("Compute shader built successfully.")

    return  compute_shader_code

def build_metal() -> str:
    global g_shader_folder

    print("Building metal shader...")
    main_metal_path = os.path.join(g_shader_folder, "kernel.metal")
    included_files = set()
    metal_shader_code = process_file(main_metal_path, included_files)

    print("Metal shader built successfully.")
    return  metal_shader_code

def make_multiline_cpp_str(shader_code : str) -> str:
    lines = shader_code.splitlines()
    cpp_lines = ['    "' + line.replace('"', '\\"') + '\\n"' for line in lines]
    return "\n".join(cpp_lines)

def make_shader_code_declare(shader_name: str, shader_type: str, shader_code: str) -> str:
    shader_str = make_multiline_cpp_str(shader_code)
    return f"const char* {shader_name}{shader_type}Code =\n{shader_str};\n"

def compile_opengl_render(args):
    global g_shader_folder
    g_shader_folder = os.path.join(args.shader_folder , 'opengl')
    output_path = args.output

    if not Path(g_shader_folder).exists():
        print(f"Error: The folder '{g_shader_folder}' does not exist.")
        sys.exit(-1)

    frag_paths = sorted(Path(g_shader_folder).glob("*.frag"))
    if len(frag_paths) == 0:
        print(f"Error: No fragment shader files found in '{g_shader_folder}'.")
        sys.exit(-1)

    program_once_code = "#pragma once\n"

    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(program_once_code)
        for frag_path in frag_paths:
            shader_name = frag_path.stem
            vert_path = Path(g_shader_folder) / f"{shader_name}.vert"
            if not vert_path.exists():
                print(f"Error: Missing vertex shader for '{frag_path.name}': '{vert_path}'.")
                sys.exit(-1)

            vert_code = build_vert(shader_name)
            frag_code = build_frag(shader_name)

            f.write(make_shader_code_declare(shader_name, "Vert", vert_code))
            f.write("\n\n")
            f.write(make_shader_code_declare(shader_name, "Frag", frag_code))
            f.write("\n\n")

def compile_opengl_compute(args):
    global g_shader_folder
    g_shader_folder = os.path.join(args.shader_folder , 'opengl')
    output_path = args.output

    if not Path(g_shader_folder).exists():
        print(f"Error: The folder '{g_shader_folder}' does not exist.")
        sys.exit(-1)
   
    compute_code = build_compute()
    compute_str = make_multiline_cpp_str(compute_code)

    compute_code_declare = f"const char* computeShaderCode =\n{compute_str};\n"
    
    #print("compute_code_declare ", compute_code_declare )

    with open(output_path, 'a', encoding='utf-8') as f:
        f.write(compute_code_declare)



def compile_metal(args):
    global g_shader_folder
    g_shader_folder = os.path.join(args.shader_folder , 'metal')
    output_path = args.output

    metal_code = build_metal()

    metal_code_str = make_multiline_cpp_str(metal_code)
    metal_code_declare = f"const char* metalCode =\n{metal_code_str};\n"

    with open(output_path, 'a', encoding='utf-8') as f:
        f.write(metal_code_declare)

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="GLSL Shader Merger Tool")
    
    parser.add_argument("--shader_folder", help="Path to the main .frag or .vert file")
    parser.add_argument("--output", help="Path to save the generated C++ header file")
    
    args = parser.parse_args()

    print("Starting shader compilation...")
    compile_opengl_render(args)
    compile_opengl_compute(args)
    compile_metal(args)
    print(f"Shader compilation completed. Output saved to: {args.output}")

    # python ./CompileShader.py --shader_folder ./  --output ./final.h
