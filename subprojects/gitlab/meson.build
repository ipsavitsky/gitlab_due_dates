project(
    'gitlab',
    'cpp',
    version: '0.1',
    default_options: ['warning_level=3', 'cpp_std=c++23'],
)

json = dependency('nlohmann_json', required: true)
cpr = dependency('cpr', required: true)

incdir = include_directories('include')

gitlab = shared_library(
    'gitlab',
    'gitlab.cpp',
    dependencies: [json, cpr],
    include_directories: incdir,
)

gitlab_dep = declare_dependency(
    include_directories: incdir,
    dependencies: [json, cpr],
    link_with: gitlab,
)
