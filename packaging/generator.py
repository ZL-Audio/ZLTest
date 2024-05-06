import os
import sys
import xml
import xml.etree.cElementTree as ET
import subprocess

def main():
    temp_dir = "./appletmp"
    subprocess.run(["mkdir", temp_dir])

    project_name = os.getenv("PROJECT_NAME", "Pamplejuce")
    product_name = os.getenv("PRODUCT_NAME", "Pamplejuce Demo")
    version = os.getenv("VERSION", "0.0.0")
    bundle_id = os.getenv("BUNDLE_ID", "")
    build_dir = os.getenv("BUILD_DIR", "")
    cert = os.getenv("APPLE_INSTALL_CERT", "")
    artifacts_name = os.getenv("ARTIFACTS_NAME", "")

    # root
    root = ET.Element("installer-gui-script", minSpecVersion="1")
    # title
    title = ET.SubElement(root, "title")
    title.text = "{} {}".format(product_name, version)
    # EULA
    if os.path.isfile("packaging/EULA"):
        eula = ET.SubElement(root, "license", file="EULA")
        eula.set("mime-type", "text/plain")
    # readme
    if os.path.isfile("packaging/Readme.rtf"):
        readme = ET.SubElement(root, "readme", file="Readme.rtf")
    # options
    options = ET.SubElement(root, "options",
                            customize="always",
                            rootVolumeOnly="true",
                            hostArchitectures="x86_64,arm64")
    # domain
    domain = ET.SubElement(root, "domain",
                           enable_anywhere="false",
                           enable_currentUserHome="false",
                           enable_localSystem="true")
    # choices outline
    outline = ET.SubElement(root, "choices-outline")

    print("Create packages")
    for plugin_format, extension, install_path in zip(
        ["VST3", "AU", "LV2", "CLAP"],
        ["vst3", "component", "lv2", "clap"],
        ["VST3", "Components", "LV2", "CLAP"]):
        if plugin_format + "_PATH" in os.environ:
            plugin_path = build_dir + "/" + os.environ[plugin_format + "_PATH"]
            if os.path.exists(plugin_path):
                identifier = "{}.{}.{}.pkg".format(bundle_id, project_name, extension)
                pkg_path = "{}/{}.{}.pkg".format(temp_dir, product_name, extension)
                command_list = [
                    "--identifier", identifier,
                    "--version", version,
                    "--component", plugin_path,
                    "--install-location", "/Library/Audio/Plug-Ins/" + install_path,
                    pkg_path]
                if len(cert) > 0:
                    command_list = ["--sign", cert] + command_list
                subprocess.run(["pkgbuild"] + command_list)
                # ET.SubElement(root, "pkg-ref", id=identifier)
                ref = ET.SubElement(root, "pkg-ref",
                                    id=identifier, version=version, onConclusion="none")
                ref.text = pkg_path
                choice = ET.SubElement(root, "choice", id=identifier,
                                       visible="true", start_selected="true",
                                       title="{} {}".format(product_name, plugin_format))
                ET.SubElement(choice, "pkg-ref", id=identifier)
                ET.SubElement(outline, "line", choice=identifier)

    # write xml
    print("")
    print("Create distribution xml")
    ET.indent(root, space="\t", level=0)
    tree = ET.ElementTree(root)
    tree.write("packaging/distribution.xml", encoding="utf-8", xml_declaration=True)

    print(ET.tostring(root, encoding='utf8').decode())

    print("")
    print("Create final package")
    command_list = ["--distribution", "packaging/distribution.xml",
                    "--package-path", temp_dir,
                    "--resources", "packaging",
                    artifacts_name + ".pkg"]
    if len(cert) > 0:
        command_list = ["--sign", cert] + command_list
        
    subprocess.run(["productbuild"] + command_list)

    if os.path.exists("packaging/icon.icns"):
        print("")
        print("Attach icns")
        with open("tmpIcon.rsrc", "w") as outfile:
            subprocess.run(["echo", "read 'icns' (-16455) \"packaging/icon.icns\";"], stdout=outfile)
        subprocess.run(["Rez -append tmpIcon.rsrc -o \"{}\".pkg".format(artifacts_name)], shell=True)
        subprocess.run(["SetFile", "-a", "C", "{}.pkg".format(artifacts_name)])
    print("")
    print("Zip package")
    subprocess.run(["zip", "{}.zip".format(artifacts_name), "{}.pkg".format(artifacts_name)])
    return 0

if __name__ == '__main__':
    sys.exit(main())