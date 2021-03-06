"""
This runs the feature testing
"""
import os
import run_cli
import compare
from config import config
from helper import helper


def main():
    """
    Main function to run feature testing
    """
    conf = config.Config()
    config_files = helper.list_files(conf.cli_configs_path, conf.config_extension)

    for config_file in config_files:
        compare_output(config_file)


def compare_output(config_path):
    """
    Compare output of one configration file
    Args:
        config_path (str): Path of configuration file
    """
    conf = config.Config()
    output_dir = conf.output_path

    helper.delete_dir(output_dir)
    helper.make_dir(output_dir)
    cli = run_cli.RunCli()
    cli.run_cli(config_path)

    file_list = []
    file_list.append(os.path.join(output_dir, conf.output_fname))
    file_list.append(os.path.splitext(config_path)[0] + ".csv")
    comp = compare.CompareOutput(file_list, os.path.basename(config_path))
    comp.compare()
    helper.delete_dir(output_dir)


main()
