rule tom10
{
  strings:
    $re = /Tom.{10,25}river|river.{10,25}Tom/
  condition:
    $re
}
