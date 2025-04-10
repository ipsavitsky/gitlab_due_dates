# Gitlab due dates

Automatically move overdue tasks a week from now.

## Building

```shell
meson setup build
meson compile -C build
```

## Confuguration

`base_url` is optional

```json
{
    "base_url": "gitlab.example.com/api/v4",
    "token": "your token goes here"
}
```
