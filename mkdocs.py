#!/usr/bin/env python
# -*- coding: utf-8 -*-
# modified from https://github.com/conanhujinming/comments-for-awesome-courses/blob/main/update.py

import os
import yaml
from urllib.parse import quote

TXT_EXTS = ['md', 'txt']
EXCLUDE_DIRS = ['.git', 'docs', '.vscode', '.circleci', 'site', 'overrides', '.github']
README_MD = ['README.md', 'readme.md', 'index.md']

TXT_URL_PREFIX = 'https://github.com/TJCS-Course/TJCS-Course/blob/main/'
BIN_URL_PREFIX = 'https://github.com/TJCS-Course/TJCS-Course/raw/main/'

def list_files(course: str):
    filelist_texts = ''
    readme_path = ''
    file_cnt = 0
    for root, dirs, files in os.walk(course):
        files.sort()
        level = root.replace(course, '').count(os.sep)
        indent = ' ' * 4 * level
        filelist_texts += '{}- {}\n'.format(indent, os.path.basename(root))
        subindent = ' ' * 4 * (level + 1)
        for f in files:
            if f not in README_MD:
                file_cnt += 1
                if f.split('.')[-1] in TXT_EXTS:
                    filelist_texts += '{}- [{}]({})\n'.format(subindent,
                                                              f, TXT_URL_PREFIX + quote('{}/{}'.format(root, f)))
                else:
                    filelist_texts += '{}- [{}]({})\n'.format(subindent,
                                                              f, BIN_URL_PREFIX + quote('{}/{}'.format(root, f)))
            elif root == course and readme_path == '':
                readme_path = '{}/{}'.format(root, f)
    if file_cnt == 0:
        return '', readme_path
    return filelist_texts, readme_path

def generate_md(term: str, course: str, readme_path: str, filelist_texts: str):
    final_texts = ['\n'.encode(), filelist_texts.encode()]
    if readme_path:
        with open(readme_path, 'rb') as file:
            final_texts = file.readlines() + final_texts

    term_path = os.path.join('docs', term)
    if not os.path.isdir(term_path):
        os.mkdir(term_path)
    with open(os.path.join(term_path, '{}.md'.format(course)), 'wb') as file:
        file.writelines(final_texts)

if __name__ == '__main__':
    if not os.path.isdir('docs'):
        os.mkdir('docs')

    # read courses from terms.yml
    with open('terms.yml', 'r') as file:
        yaml_data = yaml.safe_load(file)

    terms = yaml_data.keys()

    for term in terms:
        courses = yaml_data[term]

        for course in courses:
            filelist_texts, readme_path = list_files(course)
            generate_md(term, course, readme_path, filelist_texts)

    # use main README.md as index.md
    with open('README.md', 'rb') as file:
        mainreadme_lines = file.readlines()

    with open('docs/index.md', 'wb') as file:
        file.writelines(mainreadme_lines)

    # handle navigation part. if not specified, '大三上' will be ahead of '大二上', weird
    with open ('mkdocs.yml', 'a') as file:
        print('nav:', file=file)
        print('  - Home: index.md', file=file)

        for term in terms:
            print('  - {}:'.format(term), file=file)
            courses = yaml_data[term]
            for course in courses:
                print('    - {}: {}/{}.md'.format(course.replace('_', ' '), term, course), file=file)
