#include "reduce.h"

#include <stdexcept>

// Iterate image finding the average of each pixel and its neighbours
// Iterate again this time counting the number of neighbours around each pixel that has the same (or extremely similar) average
// Create output image
// Iterate input image copying the neighbour of each pixel with the highest number of similarly averaged neighbours

namespace vectorizer
{
    reducer::reducer(size_t reach, float threshold, float similarity)
        : _reach(reach)
        , _threshold(threshold)
        , _similarity(similarity)
        {
        }

    Image reducer::reduce_image(const Image & from) const
    {
        Image averages = averages_of(from, _reach);

        Bitmap<size_t> counts = find_similar_neighbour_averages(averages, _reach, _similarity);

        Image reduced_image = most_popular_neighbours(from, averages, counts, _reach, _threshold);

        return reduced_image;
    }

    Image reducer::averages_of(const Image & from, size_t reach)
    {
        int neighbour_min = -(int)reach;
        int neighbour_max = (int)reach;

        Image averages = Image(from.width(), from.height());

        for (int x = 0; x < averages.width(); ++x)
        {
            for (int y = 0; y < averages.height(); ++y)
            {
                pixelInt sum = pixelInt{0, 0, 0};
                int count = 0;
                for (int neighbour_x = x + neighbour_min; neighbour_x < (x + neighbour_max); ++neighbour_x)
                {
                    for (int neighbour_y = y + neighbour_min; neighbour_y < (y + neighbour_max); ++neighbour_y)
                    {
                        if (neighbour_x < 0 || 
                            neighbour_y < 0 || 
                            neighbour_x >= averages.width() ||
                            neighbour_y >= averages.height())
                        {
                            continue;
                        }
                        
                        auto neighbour = from.get(neighbour_x, neighbour_y);

                        sum += neighbour;
                        count++;
                    }
                }

                pixel average = pixel(
                    (byte)((float)sum.R / (float)count),
                    (byte)((float)sum.G / (float)count),
                    (byte)((float)sum.B / (float)count)
                );

                averages.set(x, y, average);
            }
        }

        return std::move(averages);
    }

    Bitmap<size_t> reducer::find_similar_neighbour_averages(const Image& averages, size_t reach, float similarity)
    {
        Bitmap<size_t> counts = Bitmap<size_t>(averages.width(), averages.height());

        int neighbour_min = -(int)reach;
        int neighbour_max = (int)reach;

        for (int x = 0; x < averages.width(); ++x)
        {
            for (int y = 0; y < averages.height(); ++y)
            {
                pixel my_average = averages.get(x, y);

                for (int neighbour_x = x + neighbour_min; neighbour_x < (x + neighbour_max); ++neighbour_x)
                {
                    for (int neighbour_y = y + neighbour_min; neighbour_y < (y + neighbour_max); ++neighbour_y)
                    {
                        if (neighbour_x < 0 || 
                            neighbour_y < 0 || 
                            neighbour_x >= averages.width() ||
                            neighbour_y >= averages.height())
                        {
                            continue;
                        }
                        
                        auto neighbour = averages.get(neighbour_x, neighbour_y);

                        if (my_average.is_similar_to(neighbour, similarity))
                            counts.get(x, y) += 1;
                    }
                }
            }
        }

        return std::move(counts);
    }

    Image reducer::most_popular_neighbours(const Image& from, const Image& averages, const Bitmap<size_t>& counts, size_t reach, float threshold)
    {
        if ((averages.width() != counts.width()) || (averages.height() != counts.height()))
        {
            throw std::invalid_argument("averages image not same dimensions as counts bitmap");
        }

        Image output = Image(averages.width(), averages.height());

        int neighbour_min = -(int)reach;
        int neighbour_max = (int)reach;

        for (int x = 0; x < output.width(); ++x)
        {
            for (int y = 0; y < output.height(); ++y)
            {
                int highest_count = 0;
                vector2i highest_spot = { x, y };
                float highest_difference = threshold;

                pixel my_average = averages.get(x, y);

                for (int neighbour_x = x + neighbour_min; neighbour_x < (x + neighbour_max); ++neighbour_x)
                {
                    for (int neighbour_y = y + neighbour_min; neighbour_y < (y + neighbour_max); ++neighbour_y)
                    {
                        if (neighbour_x < 0 ||
                            neighbour_y < 0 ||
                            neighbour_x >= averages.width() ||
                            neighbour_y >= averages.height())
                        {
                            continue;
                        }

                        auto neighbour_average = averages.get(neighbour_x, neighbour_y);
                        auto neighbour_popularity = counts.get(neighbour_x, neighbour_y);
                        float difference = my_average.difference_from(neighbour_average);

                        if (difference < threshold &&
                            (neighbour_popularity > highest_count))
                        {
                            highest_count = neighbour_popularity;
                            highest_spot = { neighbour_x, neighbour_y };
                            highest_difference = difference;
                        }
                    }
                }

                pixel most_popular_color = from.get(highest_spot.x, highest_spot.y);

                output.set(x, y, most_popular_color);
            }
        }

        return std::move(output);
    }
}