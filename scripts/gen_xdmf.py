from argparse import ArgumentParser
from pathlib import Path


def main():
    parser = ArgumentParser(prog="gen_xdmf.py", description="generates an xdmf file from HDF5 files written by EPPIC",
                            epilog="please see the documentation for more information")
    parser.add_argument("hdf5_path", type=str, help="path to hdf5 file written by EPPIC")

    args = parser.parse_args()

    hdf5_path = Path(args.hdf5_path)
    if ~hdf5_path.is_file():
        raise FileExistsError(f"HDF5 file `{hdf5_path}` does not exist")


if __name__ == "__main__":
    main()
