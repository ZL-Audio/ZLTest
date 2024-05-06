import os
import sys
import xml
import xml.etree.cElementTree as ET
import subprocess


def main():
    temp_dir = "./appletmp"
    subprocess.run(["mkdir", temp_dir])

    product_name = os.getenv("PRODUCT_NAME", "Pamplejuce Demo")
    version = os.getenv("VERSION", "0.0.0")
    cert = os.getenv("APPLE_INSTALLER_DEV", "")

    for plugin_format, extension, install_path in zip(
        ["VST3", "AU", "LV2"],
        ["vst3", "component", "lv2"],
        ["VST3", "Components", "LV2"]):
        if plugin_format + "_PATH" in os.environ:
            plugin_path = os.environ[plugin_format + "_PATH"]
            if os.path.exists(plugin_path):
                print("Create {} package.".format(plugin_format))
                subprocess.run("pkgbuild",
                               "--sign", cert,
                               "--identifier", "{}.{}.pkg".format(product_name, extension),
                               "--version", version,
                               "--component", plugin_path,
                               "--install-location", "/Library/Audio/Plug-Ins/" + install_path,
                               "{}/{}.{}.pkg".format(temp_dir, product_name, extension))

    return 0


if __name__ == '__main__':
    sys.exit(main())