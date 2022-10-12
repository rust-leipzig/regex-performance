rule aax
{
  strings:
    $re = /[a-q][^u-z]{13}x/
  condition:
    $re
}
