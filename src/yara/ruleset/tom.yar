rule tom
{
  strings:
    $re = /Tom|Sawyer|Huckleberry|Finn/
  condition:
    $re
}
