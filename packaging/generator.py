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
    cert = os.getenv("APPLE_INSTALLER_DEV", "")

    print(product_name + " " + version)

    for plugin_format, extension, install_path in zip(
        ["VST3", "AU", "LV2"],
        ["vst3", "component", "lv2"],
        ["VST3", "Components", "LV2"]):
        if plugin_format + "_PATH" in os.environ:
            plugin_path = build_dir + "/" + os.environ[plugin_format + "_PATH"]
            if os.path.exists(plugin_path):
                print("Create {} package.".format(plugin_format))
                subprocess.run([
                    "pkgbuild",
                    "--sign", cert,
                    "--identifier", "{}.{}.{}.pkg".format(bundle_id, project_name, extension),
                    "--version", version,
                    "--component", build_dir + "/" + plugin_path,
                    "--install-location", "/Library/Audio/Plug-Ins/" + install_path,
                    "{}/{}.{}.pkg".format(temp_dir, product_name, extension)])

    return 0


if __name__ == '__main__':
    sys.exit(main())