project(
    'gitlab_due_date',
    'cpp',
    version: '0.1',
    default_options: ['warning_level=3', 'cpp_std=c++23'],
)

spdlog = dependency('spdlog', required: true)
json = dependency('nlohmann_json', required: true)
gitlab = subproject('gitlab')
gitlab_dep = gitlab.get_variable('gitlab_dep')

executable(
    'gitlab_due_date',
    'gitlab_due_date.cpp',
    dependencies: [spdlog, json, gitlab_dep],
    install: true,
)
