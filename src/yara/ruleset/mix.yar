rule mix
{
  strings:
    $re = /["'][^"']{0,30}[?!\.]["']/
  condition:
    $re
}
