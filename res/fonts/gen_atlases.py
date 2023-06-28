from fontTools.ttLib import TTFont
import os
import subprocess

def generate_charset(font_path, charset_path):
    font = TTFont(font_path)

    # Get a list of all code points in the font
    codepoints = [c for c in font.getBestCmap().keys() if c != 10 and c != 13] # excluding newline and carriage return characters

    # Write the code points to the charset file, one code point per line
    with open(charset_path, 'w', encoding='utf-8') as f:
        for codepoint in codepoints:
            f.write(str(codepoint) + ', ')

root_directory = os.path.dirname(os.path.abspath(__file__))

for directory_name in os.listdir(root_directory):
    directory_path = os.path.join(root_directory, directory_name)
    
    # Check if it's a directory
    if os.path.isdir(directory_path):
        # Go through each file in the sub-directory
        for file_name in os.listdir(directory_path):
            # Check if it's a .ttf file
            if file_name.endswith('.ttf'):
                # Generate charset.txt for the .ttf file
                font_path = os.path.join(directory_path, file_name)
                charset_path = os.path.join(directory_path, 'charset.txt')
                generate_charset(font_path, charset_path)

                # Run msdf-atlas-gen command
                atlas_image_path = os.path.join(directory_path, 'atlas.png')
                atlas_metadata_path = os.path.join(directory_path, 'atlas.json')  # choose .json or .csv, chage flag as well
                msdf_atlas_gen_path = os.path.join(root_directory, 'msdf-atlas-gen.exe')

                command = [msdf_atlas_gen_path, '-font', font_path, '-imageout', atlas_image_path, '-json', atlas_metadata_path, '-charset', charset_path, '-type', 'msdf', '-size', '50'] # Using -size 50 here for increased glyph resolution
                
                try:
                    subprocess.run(command, check=True)
                except subprocess.CalledProcessError as e:
                    print(f"msdf-atlas-gen command failed with error: {e}")