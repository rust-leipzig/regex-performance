rule inn
{
  strings:
    $re = /([A-Za-z]awyer|[A-Za-z]inn)\s/
  condition:
    $re
}
