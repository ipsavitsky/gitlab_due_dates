# Gitlab due dates

Automatically move overdue tasks a week from now.

## Building

```shell
meson setup build
meson compile -C build
```

## Confuguration

`base_url` and the entire `ntfy` section are optional. `new_due_date` is a required field that accepts relative dates (e.g., "today", "tomorrow", "next Tuesday", "7 days") or an absolute date in YYYY-MM-DD format.

```json
{
    "base_url": "https://gitlab.com/api/v4",
    "token": "your token goes here",
    "new_due_date": "YYYY-MM-DD",
	"ntfy": {
		"url": "https://example.ntfy.sh",
		"topic": "gitlab_dd_topic",
		"token": "token_goes_here"
    }
}
```
