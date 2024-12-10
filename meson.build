project(
    'gitlab_due_date',
    'cpp',
    version: '0.1',
    default_options: ['warning_level=3', 'cpp_std=c++23'],
)

cpr = dependency('cpr', required: true)
spdlog = dependency('spdlog', required: true)
json = dependency('nlohmann_json', required: true)

exe = executable(
    'gitlab_due_date',
    'gitlab_due_date.cpp',
    dependencies: [cpr, spdlog, json],
    install: true,
)

test('basic', exe)
