## serialize
- main to save the finished task info, about the input paramters and output parameters
- can save two copy, one is string text file(about json), the others about binary file(about json)
- first read from the `task.json`, then write to the `task_backup.json`, then copy the `task_backup.json` to `task.json`
- the file format maybe

```bash
{
    "task_id1" : [{}, {}],
    "task_id2" : [{}, {}],
    ......
}
```

- fusiontask.json

```bash
{
    "task_id1" : [{input}, {output}],
    ......
}
```
- input struct
```bash
{
    "id" : "sdfdsfds",
    "algorithmkind" : 3,
    "first" : [{"filePath":"/root/test.tif", "rowNum":10, "colNum": 10, "bandNum": 10, "bitsPerPixel": 8}],
    ......
}
```

- output struct 
```bash
{
    "status" : 1,
    "first" : [0.1, 2.3, 4.5, ..., 9.8],
    ......
}
```
