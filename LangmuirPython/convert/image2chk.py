# -*- coding: utf-8 -*-
"""
@author: adam
"""
import scipy.ndimage as ndimage
import langmuir as lm
import numpy as np
import argparse
import os

desc = """
Converts an image to trap data.
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument(dest='ifile', default='image.png', type=str, nargs='?',
                        metavar='str', help='input file')
    parser.add_argument('--stub', default='sim', type=str,
                        metavar='stub', help='output file stub')
    parser.add_argument('--template', default='template.inp', type=str,
                        metavar='str', help='template input file')
    opts = parser.parse_args(args)
    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    if os.path.exists(opts.template):
        template = lm.checkpoint.load(opts.template)
        chk = lm.checkpoint.CheckPoint.from_image(opts.ifile, template)
    else:
        chk = lm.checkpoint.CheckPoint.from_image(opts.ifile)

    print chk

    handle = lm.common.format_output(stub=opts.stub, name='', ext='inp')
    chk.save(handle)
    print 'saved: %s' % handle