#!/usr/bin/env python
# -*- coding: utf-8 -*-
# modified from https://github.com/conanhujinming/comments-for-awesome-courses/blob/main/update.py

import os

EXCLUDE_DIRS = ['.git', 'docs', '.vscode', '.circleci', 'site', 'overrides', '.github']
README_MD = ['README.md', 'readme.md', 'index.md']

def list_files(course: str):
    readme_path = ''
    for root, dirs, files in os.walk(course):
        files.sort()
        for f in files:
            if root == course and readme_path == '':
                readme_path = '{}/{}'.format(root, f)
    return readme_path


def generate_md(course: str, readme_path: str, topic: str):
    final_texts = ['\n\n'.encode()]
    if readme_path:
        with open(readme_path, 'rb') as file:
            final_texts = file.readlines() + final_texts
    topic_path = os.path.join('docs', topic)
    if not os.path.isdir(topic_path):
        os.mkdir(topic_path)
    with open(os.path.join(topic_path, '{}.md'.format(course)), 'wb') as file:
        file.writelines(final_texts)
        


if __name__ == '__main__':
    if not os.path.isdir('docs'):
        os.mkdir('docs')

    topics = list(filter(lambda x: os.path.isdir(x) and (
        x not in EXCLUDE_DIRS), os.listdir('.')))  # list topics

    print(topics)

    for topic in topics:
        topic_path = os.path.join('.', topic)

        courses = list(filter(lambda x: os.path.isdir(os.path.join(topic_path, x)) and (
            x not in EXCLUDE_DIRS), os.listdir(topic_path)))  # list courses

        print(topic_path)

        for course in courses:
            course_path = os.path.join(".", topic, course)
            readme_path = list_files(course_path)
            generate_md(course, readme_path, topic)

    with open('README.md', 'rb') as file:
        mainreadme_lines = file.readlines()

    with open('docs/index.md', 'wb') as file:
        file.writelines(mainreadme_lines)
