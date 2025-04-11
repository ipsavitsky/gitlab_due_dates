# Gitlab due dates

Automatically move overdue tasks a week from now.

## Building

```shell
meson setup build
meson compile -C build
```

## Confuguration

`base_url` and the entire `ntfy` section are optional

```json
{
    "base_url": "gitlab.example.com/api/v4",
    "token": "your token goes here",
	"ntfy": {
		"url": "example.ntfy.sh",
		"topic": "gitlab_dd_topic",
		"token": "token_goes_here"
    }
}
```
